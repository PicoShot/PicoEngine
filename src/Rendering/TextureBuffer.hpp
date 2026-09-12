#pragma once
#include "Buffer.hpp"

namespace PicoEngine::Rendering
{
// A typed, read-only texel buffer (samplerBuffer in GLSL), not an image or staging buffer.
class TextureBuffer
{
  public:
    TextureBuffer(Device& device, VkFormat format, std::span<const std::byte> texels);
    ~TextureBuffer();
    TextureBuffer(const TextureBuffer&)            = delete;
    TextureBuffer& operator=(const TextureBuffer&) = delete;
    VkBufferView   View() const
    {
        return m_view;
    }
    const Buffer& Storage() const
    {
        return *m_buffer;
    }

  private:
    Device&                 m_device;
    std::unique_ptr<Buffer> m_buffer;
    VkBufferView            m_view = VK_NULL_HANDLE;
};
} // namespace PicoEngine::Rendering
