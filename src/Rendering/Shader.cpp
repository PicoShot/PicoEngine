#include "Shader.hpp"

namespace PicoEngine::Rendering
{
Shader::Shader(Device& device, const std::filesystem::path& path) : m_device(device)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error(std::format("Cannot open shader: {}", path.string()));
    const auto size = file.tellg();
    if (size < 20 || size % 4 != 0) throw std::runtime_error("Invalid SPIR-V size");
    std::vector<uint32_t> code(static_cast<size_t>(size) / 4);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(code.data()), size) || code.front() != 0x07230203)
        throw std::runtime_error("Invalid SPIR-V shader");
    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize = static_cast<size_t>(size);
    info.pCode    = code.data();
    Check(vkCreateShaderModule(device.Handle(), &info, nullptr, &m_module), "Create shader module");
}
Shader::~Shader()
{
    vkDestroyShaderModule(m_device.Handle(), m_module, nullptr);
}
} // namespace PicoEngine::Rendering
