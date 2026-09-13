#pragma once
#include "Texture.hpp"

namespace PicoEngine::Rendering
{
class Texture2D final : public Texture
{
  public:
    Texture2D(Device& device, uint32_t width, uint32_t height, VkFormat format,
              VkImageUsageFlags usage, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT)
        : Texture(device, {width, height, 1}, format, usage, aspect)
    {
    }
    static std::unique_ptr<Texture2D> Upload(Device& device, uint32_t width, uint32_t height,
                                             std::span<const std::byte> pixels);
};
} // namespace PicoEngine::Rendering
