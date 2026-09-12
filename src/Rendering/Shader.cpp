#include "Shader.hpp"

namespace PicoEngine::Rendering
{
Shader::Shader(Device& device, std::span<const uint32_t> spirv, std::string_view debugName) : m_device(device)
{
    const std::string name(debugName);
    PICO_ASSERT(!spirv.empty() && spirv.size() * sizeof(uint32_t) >= 20, "Invalid SPIR-V size: {}", name);
    PICO_ASSERT(spirv.front() == 0x07230203, "Invalid SPIR-V shader: {}", name);
    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize = spirv.size_bytes();
    info.pCode    = spirv.data();
    Check(vkCreateShaderModule(device.Handle(), &info, nullptr, &m_module), "Create shader module");
    LOG_DEBUG("Created shader module '{}' ({} words)", name, spirv.size());
}
Shader::~Shader()
{
    vkDestroyShaderModule(m_device.Handle(), m_module, nullptr);
}
} // namespace PicoEngine::Rendering
