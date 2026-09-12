#include "TextureBuffer.hpp"

namespace PicoEngine::Rendering
{
TextureBuffer::TextureBuffer(Device& device, VkFormat format, std::span<const std::byte> texels) : m_device(device)
{
    // Keep the initial format contract explicit; more formats can be added with their byte sizes.
    uint32_t texelSize = 0;
    switch (format)
    {
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
        texelSize = 4;
        break;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        texelSize = 16;
        break;
    default:
        PICO_ASSERT_FAIL("Unsupported TextureBuffer format: {}", static_cast<int>(format));
    }
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device.Physical(), format, &properties);
    VkPhysicalDeviceProperties limits;
    vkGetPhysicalDeviceProperties(device.Physical(), &limits);
    PICO_ASSERT((properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) && !texels.empty() &&
                texels.size() % texelSize == 0 && texels.size() / texelSize <= limits.limits.maxTexelBufferElements,
                "Invalid or unsupported texel buffer");
    m_buffer = Buffer::Upload(device, texels, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    VkBufferViewCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
    info.buffer = m_buffer->Handle();
    info.format = format;
    info.range  = texels.size();
    Check(vkCreateBufferView(device.Handle(), &info, nullptr, &m_view), "Create texel buffer view");
}
TextureBuffer::~TextureBuffer()
{
    vkDestroyBufferView(m_device.Handle(), m_view, nullptr);
}
} // namespace PicoEngine::Rendering
