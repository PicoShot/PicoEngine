#pragma once
#include "Rendering/Texture2D.hpp"

namespace PicoEngine::Rendering
{
class Swapchain
{
  public:
    Swapchain(Device& device, VkExtent2D requestedExtent);
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

  private:
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
};
} // namespace PicoEngine::Rendering
