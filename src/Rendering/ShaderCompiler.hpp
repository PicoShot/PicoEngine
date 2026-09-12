#pragma once

namespace PicoEngine::Rendering
{
enum class ShaderStage
{
    Vertex,
    Fragment,
    Compute
};

struct CompiledShader
{
    std::vector<uint32_t>              spirv;
    std::vector<std::filesystem::path> dependencies;
};

class ShaderCompiler
{
  public:
    ShaderCompiler();
    ShaderCompiler(const ShaderCompiler&)            = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;

    void AddIncludeDir(std::filesystem::path dir);

    CompiledShader CompileFile(const std::filesystem::path& path);
    CompiledShader CompileFile(const std::filesystem::path& path, ShaderStage stage);
    CompiledShader CompileSource(std::string_view debugName, std::string_view source, ShaderStage stage,
                                 const std::filesystem::path& includeDir = {});

  private:
    shaderc::Compiler                  m_compiler;
    std::vector<std::filesystem::path> m_includeDirs;
};
} // namespace PicoEngine::Rendering
