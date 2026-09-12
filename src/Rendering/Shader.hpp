#pragma once
#include "Rendering/Vulkan/Device.hpp"

namespace PicoEngine::Rendering
{
class Shader
{
  public:
    Shader(Device& device, const std::filesystem::path& path);
    ~Shader();
    Shader(const Shader&)                   = delete;
    Shader&        operator=(const Shader&) = delete;
    VkShaderModule Handle() const
    {
        return m_module;
    }

  private:
    Device&        m_device;
    VkShaderModule m_module = VK_NULL_HANDLE;
};
} // namespace PicoEngine::Rendering
