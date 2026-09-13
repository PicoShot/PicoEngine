#pragma once

namespace PicoEngine::IO
{
class Vfs;
}

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
    std::vector<uint32_t>    spirv;
    std::vector<std::string> dependencies;
};

class ShaderCompiler
{
  public:
    explicit ShaderCompiler(const IO::Vfs& vfs);
    ShaderCompiler(const ShaderCompiler&)            = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;

    void AddIncludePrefix(std::string prefix);

    CompiledShader CompileFile(std::string_view virtualPath);
    CompiledShader CompileFile(std::string_view virtualPath, ShaderStage stage);
    CompiledShader CompileSource(std::string_view debugName, std::string_view source, ShaderStage stage,
                                 std::string_view includeDir = {});

  private:
    const IO::Vfs&           m_vfs;
    shaderc::Compiler        m_compiler;
    std::vector<std::string> m_includePrefixes;
};
} // namespace PicoEngine::Rendering
