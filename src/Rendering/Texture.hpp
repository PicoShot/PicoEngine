#pragma once
#include "Rendering/Vulkan/Device.hpp"

namespace PicoEngine::Rendering
{
// Image storage and its default view. Layout transitions belong to command recording.
class Texture
{
  public:
    virtual ~Texture();
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    VkImage  Handle() const
    {
        return m_image;
    }
    VkImageView View() const
    {
        return m_view;
    }
    VkFormat Format() const
    {
        return m_format;
    }
    VkExtent3D Extent() const
    {
        return m_extent;
    }

  protected:
    Texture(Device& device, VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);

  private:
    Device&        m_device;
    VkImage        m_image  = VK_NULL_HANDLE;
    VkImageView    m_view   = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkFormat       m_format;
    VkExtent3D     m_extent;
};
} // namespace PicoEngine::Rendering
