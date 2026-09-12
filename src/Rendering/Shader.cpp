#include "Shader.hpp"

namespace PicoEngine::Rendering
{
Shader::Shader(Device& device, const std::filesystem::path& path) : m_device(device)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    PICO_ASSERT(file, "Cannot open shader: {}", path.string());
    const auto size = file.tellg();
    PICO_ASSERT(size >= 20 && size % 4 == 0, "Invalid SPIR-V size: {}", path.string());
    std::vector<uint32_t> code(static_cast<size_t>(size) / 4);
    file.seekg(0);
    PICO_ASSERT(file.read(reinterpret_cast<char*>(code.data()), size) && code.front() == 0x07230203,
                "Invalid SPIR-V shader: {}", path.string());
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
