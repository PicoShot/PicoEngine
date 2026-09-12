#include "Buffer.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine::Rendering
{
Buffer::Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory)
    : m_device(device), m_size(size)
{
    PICO_ASSERT(size > 0, "Buffer must not be empty");
    // Write uses coherent memory to avoid exposing flush alignment to callers.
    if (memory & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) memory |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    try
    {
        VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        info.size  = size;
        info.usage = usage;
        Check(vkCreateBuffer(device.Handle(), &info, nullptr, &m_buffer), "Create buffer");
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(device.Handle(), m_buffer, &requirements);
        VkMemoryAllocateInfo alloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        alloc.allocationSize  = requirements.size;
        alloc.memoryTypeIndex = device.FindMemory(requirements.memoryTypeBits, memory);
        Check(vkAllocateMemory(device.Handle(), &alloc, nullptr, &m_memory), "Allocate buffer memory");
        Check(vkBindBufferMemory(device.Handle(), m_buffer, m_memory, 0), "Bind buffer memory");
        if (memory & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            Check(vkMapMemory(device.Handle(), m_memory, 0, VK_WHOLE_SIZE, 0, &m_mapped), "Map buffer");
    }
    catch (...)
    {
        if (m_buffer) vkDestroyBuffer(device.Handle(), m_buffer, nullptr);
        if (m_memory) vkFreeMemory(device.Handle(), m_memory, nullptr);
        throw;
    }
}
Buffer::~Buffer()
{
    if (m_mapped) vkUnmapMemory(m_device.Handle(), m_memory);
    vkDestroyBuffer(m_device.Handle(), m_buffer, nullptr);
    vkFreeMemory(m_device.Handle(), m_memory, nullptr);
}
void Buffer::Write(std::span<const std::byte> bytes, VkDeviceSize offset)
{
    PICO_ASSERT(m_mapped && offset <= m_size && bytes.size() <= m_size - offset, "Invalid buffer write");
    std::memcpy(static_cast<std::byte*>(m_mapped) + offset, bytes.data(), bytes.size());
}
std::unique_ptr<Buffer> Buffer::Upload(Device& device, std::span<const std::byte> bytes, VkBufferUsageFlags usage)
{
    Buffer staging(device, bytes.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    staging.Write(bytes);
    auto target = std::make_unique<Buffer>(device, bytes.size(), usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    device.Immediate([&](VkCommandBuffer command) {
        VkBufferCopy copy{0, 0, bytes.size()};
        vkCmdCopyBuffer(command, staging.Handle(), target->Handle(), 1, &copy);
        VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
    });
    return target;
}
} // namespace PicoEngine::Rendering
