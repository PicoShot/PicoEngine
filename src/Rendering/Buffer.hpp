#pragma once
#include "Rendering/Vulkan/Device.hpp"

namespace PicoEngine::Rendering
{
class Buffer
{
  public:
    Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
           VkMemoryPropertyFlags memory = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    ~Buffer();
    Buffer(const Buffer&)             = delete;
    Buffer&  operator=(const Buffer&) = delete;
    VkBuffer Handle() const
    {
        return m_buffer;
    }
    VkDeviceSize Size() const
    {
        return m_size;
    }
    // Host-visible memory only. Caller must ensure the GPU is not using this range.
    void Write(std::span<const std::byte> bytes, VkDeviceSize offset = 0);
    // Synchronous startup upload, not intended for per-frame streaming.
    static std::unique_ptr<Buffer> Upload(Device& device, std::span<const std::byte> bytes, VkBufferUsageFlags usage);

  private:
    Device&        m_device;
    VkBuffer       m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkDeviceSize   m_size;
    void*          m_mapped = nullptr;
};
} // namespace PicoEngine::Rendering
