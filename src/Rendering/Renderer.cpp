#include "Renderer.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine::Rendering
{
Renderer::Renderer(SDL_Window* window) : m_window(window), m_device(window)
{
    try
    {
        for (auto& frame : m_frames)
        {
            VkCommandBufferAllocateInfo alloc{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            alloc.commandPool        = m_device.CommandPool();
            alloc.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc.commandBufferCount = 1;
            Check(vkAllocateCommandBuffers(m_device.Handle(), &alloc, &frame.command), "Allocate frame command");
            VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            Check(vkCreateSemaphore(m_device.Handle(), &semaphore, nullptr, &frame.acquired), "Create acquire semaphore");
            VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            Check(vkCreateFence(m_device.Handle(), &fence, nullptr, &frame.fence), "Create frame fence");
        }
        LOG_DEBUG("Renderer initialized");
    }
    catch (const std::exception& exception)
    {
        DestroyFrames();
        PICO_ASSERT_FAIL("Renderer initialization failed: {}", exception.what());
    }
}
Renderer::~Renderer()
{
    vkDeviceWaitIdle(m_device.Handle());
    DestroyFrames();
    m_swapchain.reset();
    LOG_DEBUG("Renderer shut down");
}
void Renderer::DestroyFrames() noexcept
{
    for (auto& frame : m_frames)
    {
        if (frame.fence) vkDestroyFence(m_device.Handle(), frame.fence, nullptr);
        if (frame.acquired) vkDestroySemaphore(m_device.Handle(), frame.acquired, nullptr);
        if (frame.command) vkFreeCommandBuffers(m_device.Handle(), m_device.CommandPool(), 1, &frame.command);
    }
}
VkCommandBuffer Renderer::BeginFrame()
{
    PICO_ASSERT(!m_active, "Frame already active");
    int width = 0, height = 0;
    PICO_ASSERT(SDL_GetWindowSizeInPixels(m_window, &width, &height), "Failed to get window size: {}", SDL_GetError());
    if (width <= 0 || height <= 0 || (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED))
    {
        SDL_Delay(16);
        return VK_NULL_HANDLE;
    }
    VkExtent2D extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    if (m_recreate || extent.width != m_requestedExtent.width || extent.height != m_requestedExtent.height)
    {
        m_device.WaitIdle();
        // Destroy first: a native window must not have two non-retired swapchains.
        m_swapchain.reset();
        m_swapchain       = std::make_unique<Swapchain>(m_device, extent, m_vsync);
        m_requestedExtent = extent;
        m_recreate        = false;
        ++m_generation;
    }
    if (m_swapchain->HasPendingPresent())
        m_swapchain->WaitForPreviousPresent(1000000000);
    auto& frame = m_frames[m_frame];
    Check(vkWaitForFences(m_device.Handle(), 1, &frame.fence, VK_TRUE, UINT64_MAX), "Wait for frame");
    auto result = vkAcquireNextImageKHR(m_device.Handle(), m_swapchain->Handle(), UINT64_MAX, frame.acquired, VK_NULL_HANDLE, &m_image);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_recreate = true;
        return VK_NULL_HANDLE;
    }
    if (result == VK_SUBOPTIMAL_KHR)
        m_recreate = true;
    else
        Check(result, "Acquire image");
    Check(vkResetCommandBuffer(frame.command, 0), "Reset frame command");
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    Check(vkBeginCommandBuffer(frame.command, &begin), "Begin frame");
    std::array<VkClearValue, 2> clears{};
    clears[0].color        = {{0.025f, 0.035f, 0.055f, 1.0f}};
    clears[1].depthStencil = {1.0f, 0};
    VkRenderPassBeginInfo pass{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    pass.renderPass        = m_swapchain->RenderPass();
    pass.framebuffer       = m_swapchain->Framebuffer(m_image);
    pass.renderArea.extent = m_swapchain->Extent();
    pass.clearValueCount   = static_cast<uint32_t>(clears.size());
    pass.pClearValues      = clears.data();
    vkCmdBeginRenderPass(frame.command, &pass, VK_SUBPASS_CONTENTS_INLINE);
    extent = m_swapchain->Extent();
    VkViewport viewport{0, 0, static_cast<float>(extent.width), static_cast<float>(extent.height), 0, 1};
    VkRect2D   scissor{{0, 0}, extent};
    vkCmdSetViewport(frame.command, 0, 1, &viewport);
    vkCmdSetScissor(frame.command, 0, 1, &scissor);
    m_active = true;
    return frame.command;
}
void Renderer::SetVsync(bool enabled)
{
    if (enabled == m_vsync)
        return;
    m_vsync    = enabled;
    m_recreate = true;
    LOG_DEBUG("Vsync {}", enabled ? "enabled" : "disabled");
}
void Renderer::EndFrame()
{
    PICO_ASSERT(m_active, "No active frame");
    auto& frame = m_frames[m_frame];
    vkCmdEndRenderPass(frame.command);
    Check(vkEndCommandBuffer(frame.command), "End frame");
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSemaphore          present   = m_swapchain->PresentSemaphore(m_image);
    VkSubmitInfo         submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.waitSemaphoreCount   = 1;
    submit.pWaitSemaphores      = &frame.acquired;
    submit.pWaitDstStageMask    = &waitStage;
    submit.commandBufferCount   = 1;
    submit.pCommandBuffers      = &frame.command;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores    = &present;
    Check(vkResetFences(m_device.Handle(), 1, &frame.fence), "Reset frame fence");
    Check(vkQueueSubmit(m_device.Queue(), 1, &submit, frame.fence), "Submit frame");
    VkSwapchainKHR   swapchain = m_swapchain->Handle();
    VkPresentInfoKHR info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores    = &present;
    info.swapchainCount     = 1;
    info.pSwapchains        = &swapchain;
    info.pImageIndices      = &m_image;
    VkPresentIdKHR presentIdInfo{VK_STRUCTURE_TYPE_PRESENT_ID_KHR};
    uint64_t       presentId = 0;
    if (m_device.IsExtensionEnabled(VK_KHR_PRESENT_WAIT_EXTENSION_NAME))
    {
        presentId                    = m_swapchain->TakePresentId();
        presentIdInfo.swapchainCount = 1;
        presentIdInfo.pPresentIds    = &presentId;
        info.pNext                   = &presentIdInfo;
    }
    auto result = vkQueuePresentKHR(m_device.Queue(), &info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        m_recreate = true;
    else
        Check(result, "Present frame");
    m_active = false;
    m_frame  = (m_frame + 1) % static_cast<uint32_t>(m_frames.size());
}
} // namespace PicoEngine::Rendering
