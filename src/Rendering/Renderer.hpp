#pragma once
#include "Rendering/Vulkan/Swapchain.hpp"

namespace PicoEngine::Rendering
{
// Single render thread. BeginFrame opens the default color/depth pass; EndFrame presents it.
class Renderer
{
  public:
    explicit Renderer(SDL_Window* window);
    ~Renderer();
    Renderer(const Renderer&)                  = delete;
    Renderer&       operator=(const Renderer&) = delete;
    VkCommandBuffer BeginFrame(); // null when minimized or recreating an out-of-date surface
    void            EndFrame();
    Device&         GetDevice()
    {
        return m_device;
    }
    VkRenderPass RenderPass() const
    {
        return m_swapchain->RenderPass();
    }
    VkExtent2D Extent() const
    {
        return m_swapchain->Extent();
    }
    uint64_t Generation() const
    {
        return m_generation;
    }

  private:
    struct Frame
    {
        VkCommandBuffer command  = VK_NULL_HANDLE;
        VkSemaphore     acquired = VK_NULL_HANDLE;
        VkFence         fence    = VK_NULL_HANDLE;
    };
    void                       DestroyFrames() noexcept;
    SDL_Window*                m_window;
    Device                     m_device;
    std::unique_ptr<Swapchain> m_swapchain;
    std::array<Frame, 2>       m_frames{};
    uint32_t                   m_frame      = 0;
    uint32_t                   m_image      = 0;
    uint64_t                   m_generation = 0;
    VkExtent2D                 m_requestedExtent{};
    bool                       m_recreate = true;
    bool                       m_active   = false;
};
} // namespace PicoEngine::Rendering
