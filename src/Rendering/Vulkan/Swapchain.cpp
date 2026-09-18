#include "Swapchain.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine::Rendering
{
namespace
{
const char*      PresentModeName(VkPresentModeKHR mode) noexcept;
VkPresentModeKHR ChoosePresentMode(Device& device, bool vsync)
{
    uint32_t count = 0;
    Check(vkGetPhysicalDeviceSurfacePresentModesKHR(device.Physical(), device.Surface(), &count, nullptr),
          "Query present modes");
    std::vector<VkPresentModeKHR> modes(count);
    Check(vkGetPhysicalDeviceSurfacePresentModesKHR(device.Physical(), device.Surface(), &count, modes.data()),
          "Query present modes");
    PICO_ASSERT(!modes.empty(), "Surface has no present modes");
    std::string reported;
    for (const auto reportedMode : modes)
    {
        if (!reported.empty())
            reported += '|';
        reported += std::format("{}({})", PresentModeName(reportedMode), static_cast<int>(reportedMode));
    }
    LOG_DEBUG("Supported present modes: {}", reported);
    const auto supported = [&](VkPresentModeKHR candidate) { return std::find(modes.begin(), modes.end(), candidate) != modes.end(); };

    if (vsync)
    {
        if (supported(VK_PRESENT_MODE_MAILBOX_KHR))
            return VK_PRESENT_MODE_MAILBOX_KHR;
        if (device.IsExtensionEnabled(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME) &&
            supported(VK_PRESENT_MODE_FIFO_LATEST_READY_EXT))
            return VK_PRESENT_MODE_FIFO_LATEST_READY_EXT;
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    if (supported(VK_PRESENT_MODE_IMMEDIATE_KHR))
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    if (supported(VK_PRESENT_MODE_FIFO_RELAXED_KHR))
        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    return VK_PRESENT_MODE_FIFO_KHR;
}
const char* PresentModeName(VkPresentModeKHR mode) noexcept
{
    switch (mode)
    {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        return "Immediate";
    case VK_PRESENT_MODE_MAILBOX_KHR:
        return "Mailbox";
    case VK_PRESENT_MODE_FIFO_KHR:
        return "Fifo";
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        return "FifoRelaxed";
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
        return "SharedDemandRefresh";
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
        return "SharedContinuousRefresh";
    case VK_PRESENT_MODE_FIFO_LATEST_READY_EXT:
        return "FifoLatestReady";
    default:
        return "Unknown";
    }
}
} // namespace
Swapchain::Swapchain(Device& device, VkExtent2D requestedExtent, bool vsync) : m_device(device)
{
    try
    {
        VkSurfaceCapabilitiesKHR caps;
        Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.Physical(), device.Surface(), &caps), "Query surface");
        uint32_t count = 0;
        Check(vkGetPhysicalDeviceSurfaceFormatsKHR(device.Physical(), device.Surface(), &count, nullptr), "Query surface formats");
        std::vector<VkSurfaceFormatKHR> formats(count);
        Check(vkGetPhysicalDeviceSurfaceFormatsKHR(device.Physical(), device.Surface(), &count, formats.data()), "Query surface formats");
        PICO_ASSERT(!formats.empty(), "Surface has no formats");
        auto format = formats.front();
        if (format.format == VK_FORMAT_UNDEFINED) format = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        for (auto candidate : formats)
            if (candidate.format == VK_FORMAT_B8G8R8A8_SRGB && candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) format = candidate;
        m_extent = caps.currentExtent;
        if (m_extent.width == std::numeric_limits<uint32_t>::max())
            m_extent = {std::clamp(requestedExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width),
                        std::clamp(requestedExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height)};
        PICO_ASSERT(m_extent.width && m_extent.height, "Surface has zero extent");
        PICO_ASSERT(caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "Surface cannot be a color attachment");
        uint32_t imageCount = caps.minImageCount + 1;
        if (caps.maxImageCount) imageCount = std::min(imageCount, caps.maxImageCount);
        VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        info.surface          = device.Surface();
        info.minImageCount    = imageCount;
        info.imageFormat      = format.format;
        info.imageColorSpace  = format.colorSpace;
        info.imageExtent      = m_extent;
        info.imageArrayLayers = 1;
        info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.preTransform     = caps.currentTransform;
        for (auto alpha : {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                           VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR})
            if (caps.supportedCompositeAlpha & alpha)
            {
                info.compositeAlpha = alpha;
                break;
            }
        const VkPresentModeKHR presentMode = ChoosePresentMode(device, vsync);
        info.presentMode                   = presentMode;
        info.clipped                       = VK_TRUE;
        Check(vkCreateSwapchainKHR(device.Handle(), &info, nullptr, &m_swapchain), "Create swapchain");
        Check(vkGetSwapchainImagesKHR(device.Handle(), m_swapchain, &count, nullptr), "Query swapchain images");
        std::vector<VkImage> images(count);
        Check(vkGetSwapchainImagesKHR(device.Handle(), m_swapchain, &count, images.data()), "Query swapchain images");
        VkFormat depthFormat = VK_FORMAT_UNDEFINED;
        for (auto candidate : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM})
        {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(device.Physical(), candidate, &properties);
            if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                depthFormat = candidate;
                break;
            }
        }
        PICO_ASSERT(depthFormat != VK_FORMAT_UNDEFINED, "No supported depth format");
        std::array<VkAttachmentDescription, 2> attachments{};
        attachments[0].format         = format.format;
        attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachments[1].format         = depthFormat;
        attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference color{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depth{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        VkSubpassDescription  subpass{};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &color;
        subpass.pDepthStencilAttachment = &depth;
        VkSubpassDependency dependency{};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask  = dependency.srcStageMask;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        VkRenderPassCreateInfo pass{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        pass.attachmentCount = static_cast<uint32_t>(attachments.size());
        pass.pAttachments    = attachments.data();
        pass.subpassCount    = 1;
        pass.pSubpasses      = &subpass;
        pass.dependencyCount = 1;
        pass.pDependencies   = &dependency;
        Check(vkCreateRenderPass(device.Handle(), &pass, nullptr, &m_renderPass), "Create render pass");
        m_views.resize(count);
        m_framebuffers.resize(count);
        m_presentSemaphores.resize(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view.image            = images[i];
            view.viewType         = VK_IMAGE_VIEW_TYPE_2D;
            view.format           = format.format;
            view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            Check(vkCreateImageView(device.Handle(), &view, nullptr, &m_views[i]), "Create swapchain view");
            m_depth.push_back(std::make_unique<Texture2D>(device, m_extent.width, m_extent.height, depthFormat,
                                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT));
            std::array<VkImageView, 2> views{m_views[i], m_depth.back()->View()};
            VkFramebufferCreateInfo    framebuffer{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            framebuffer.renderPass      = m_renderPass;
            framebuffer.attachmentCount = static_cast<uint32_t>(views.size());
            framebuffer.pAttachments    = views.data();
            framebuffer.width           = m_extent.width;
            framebuffer.height          = m_extent.height;
            framebuffer.layers          = 1;
            Check(vkCreateFramebuffer(device.Handle(), &framebuffer, nullptr, &m_framebuffers[i]), "Create framebuffer");
            VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            Check(vkCreateSemaphore(device.Handle(), &semaphore, nullptr, &m_presentSemaphores[i]), "Create presentation semaphore");
        }
        LOG_DEBUG("Swapchain created: {}x{}, {} images, present mode {} (present-wait {})", m_extent.width,
                  m_extent.height, count, PresentModeName(presentMode),
                  m_device.IsExtensionEnabled(VK_KHR_PRESENT_WAIT_EXTENSION_NAME) ? "on" : "off");
    }
    catch (const std::exception& exception)
    {
        Destroy();
        PICO_ASSERT_FAIL("Swapchain creation failed: {}", exception.what());
    }
}
Swapchain::~Swapchain()
{
    Destroy();
}
void Swapchain::WaitForPreviousPresent(uint64_t timeoutNanoseconds) const
{
    if (m_lastPresentId == kNoPresentId || !m_device.IsExtensionEnabled(VK_KHR_PRESENT_WAIT_EXTENSION_NAME))
        return;

    const VkResult result = m_device.WaitForPresent(m_swapchain, m_lastPresentId, timeoutNanoseconds);
    if (result == VK_SUCCESS || result == VK_TIMEOUT || result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_ERROR_SURFACE_LOST_KHR)
        return;
    Check(result, "Wait for present");
}
void Swapchain::Destroy() noexcept
{
    for (auto framebuffer : m_framebuffers)
        if (framebuffer) vkDestroyFramebuffer(m_device.Handle(), framebuffer, nullptr);
    for (auto semaphore : m_presentSemaphores)
        if (semaphore) vkDestroySemaphore(m_device.Handle(), semaphore, nullptr);
    m_depth.clear();
    for (auto view : m_views)
        if (view) vkDestroyImageView(m_device.Handle(), view, nullptr);
    if (m_renderPass) vkDestroyRenderPass(m_device.Handle(), m_renderPass, nullptr);
    if (m_swapchain) vkDestroySwapchainKHR(m_device.Handle(), m_swapchain, nullptr);
}
} // namespace PicoEngine::Rendering
