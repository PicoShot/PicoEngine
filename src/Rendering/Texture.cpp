#include "Texture.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine::Rendering
{
Texture::Texture(Device& device, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect)
    : m_device(device), m_format(format), m_extent(extent)
{
    PICO_ASSERT(extent.width && extent.height && extent.depth == 1, "Invalid 2D texture extent");
    try
    {
        VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        info.imageType   = VK_IMAGE_TYPE_2D;
        info.extent      = extent;
        info.mipLevels   = 1;
        info.arrayLayers = 1;
        info.format      = format;
        info.tiling      = VK_IMAGE_TILING_OPTIMAL;
        info.usage       = usage;
        info.samples     = VK_SAMPLE_COUNT_1_BIT;
        Check(vkCreateImage(device.Handle(), &info, nullptr, &m_image), "Create texture");
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device.Handle(), m_image, &requirements);
        VkMemoryAllocateInfo alloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        alloc.allocationSize  = requirements.size;
        alloc.memoryTypeIndex = device.FindMemory(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        Check(vkAllocateMemory(device.Handle(), &alloc, nullptr, &m_memory), "Allocate texture memory");
        Check(vkBindImageMemory(device.Handle(), m_image, m_memory, 0), "Bind texture memory");
        VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view.image            = m_image;
        view.viewType         = VK_IMAGE_VIEW_TYPE_2D;
        view.format           = format;
        view.subresourceRange = {aspect, 0, 1, 0, 1};
        Check(vkCreateImageView(device.Handle(), &view, nullptr, &m_view), "Create texture view");
    }
    catch (...)
    {
        if (m_image) vkDestroyImage(device.Handle(), m_image, nullptr);
        if (m_memory) vkFreeMemory(device.Handle(), m_memory, nullptr);
        throw;
    }
}
Texture::~Texture()
{
    vkDestroyImageView(m_device.Handle(), m_view, nullptr);
    vkDestroyImage(m_device.Handle(), m_image, nullptr);
    vkFreeMemory(m_device.Handle(), m_memory, nullptr);
}
} // namespace PicoEngine::Rendering
