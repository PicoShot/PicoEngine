#pragma once

namespace PicoEngine::Rendering
{
void Check(VkResult result, std::string_view operation);

// Owns the Vulkan instance, presentation surface, device and upload command pool.
// All GPU resources must be destroyed before the device, on the render thread.
class Device
{
  public:
    explicit Device(SDL_Window* window);
    ~Device();
    Device(const Device&)            = delete;
    Device& operator=(const Device&) = delete;

    VkDevice Handle() const
    {
        return m_device;
    }
    VkPhysicalDevice Physical() const
    {
        return m_physical;
    }
    VkSurfaceKHR Surface() const
    {
        return m_surface;
    }
    VkQueue Queue() const
    {
        return m_queue;
    }
    uint32_t QueueFamily() const
    {
        return m_queueFamily;
    }
    VkCommandPool CommandPool() const
    {
        return m_pool;
    }
    uint32_t FindMemory(uint32_t bits, VkMemoryPropertyFlags properties) const;
    void     Immediate(const std::function<void(VkCommandBuffer)>& record);
    void     WaitIdle() const;

  private:
    void                     Destroy() noexcept;
    VkInstance               m_instance    = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debug       = VK_NULL_HANDLE;
    VkSurfaceKHR             m_surface     = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physical    = VK_NULL_HANDLE;
    VkDevice                 m_device      = VK_NULL_HANDLE;
    VkQueue                  m_queue       = VK_NULL_HANDLE;
    uint32_t                 m_queueFamily = 0;
    VkCommandPool            m_pool        = VK_NULL_HANDLE;
};
} // namespace PicoEngine::Rendering
