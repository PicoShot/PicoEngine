#pragma once
#include "Rendering/Texture2D.hpp"

namespace PicoEngine::Rendering
{
class Swapchain
{
  public:
    Swapchain(Device& device, VkExtent2D requestedExtent, bool vsync);
    ~Swapchain();
    Swapchain(const Swapchain&)                = delete;
    Swapchain&     operator=(const Swapchain&) = delete;
    VkSwapchainKHR Handle() const
    {
        return m_swapchain;
    }
    VkExtent2D Extent() const
    {
        return m_extent;
    }
    VkRenderPass RenderPass() const
    {
        return m_renderPass;
    }
    VkFramebuffer Framebuffer(uint32_t image) const
    {
        return m_framebuffers.at(image);
    }
    VkSemaphore PresentSemaphore(uint32_t image) const
    {
        return m_presentSemaphores.at(image);
    }
    uint64_t TakePresentId()
    {
        const uint64_t id = m_nextPresentId++;
        m_lastPresentId   = id;
        return id;
    }
    bool HasPendingPresent() const noexcept
    {
        return m_lastPresentId != 0;
    }
    void WaitForPreviousPresent(uint64_t timeoutNanoseconds) const;

  private:
    static constexpr uint64_t               kNoPresentId = 0;
    void                                    Destroy() noexcept;
    Device&                                 m_device;
    VkSwapchainKHR                          m_swapchain = VK_NULL_HANDLE;
    VkExtent2D                              m_extent{};
    VkRenderPass                            m_renderPass = VK_NULL_HANDLE;
    std::vector<VkImageView>                m_views;
    std::vector<std::unique_ptr<Texture2D>> m_depth;
    std::vector<VkFramebuffer>              m_framebuffers;
    // Presentation consumes these asynchronously: use one per swapchain image, not per frame.
    std::vector<VkSemaphore> m_presentSemaphores;
    uint64_t                 m_nextPresentId = 1;
    uint64_t                 m_lastPresentId = kNoPresentId;
};
} // namespace PicoEngine::Rendering
