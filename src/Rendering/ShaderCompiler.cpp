#include "ShaderCompiler.hpp"
#include "Debug/Debug.hpp"
#include "IO/Vfs.hpp"

namespace PicoEngine::Rendering
{
namespace
{
constexpr const char* kEntryPoint = "main";

shaderc_shader_kind ToKind(ShaderStage stage)
{
    switch (stage)
    {
    case ShaderStage::Vertex:
        return shaderc_glsl_vertex_shader;
    case ShaderStage::Fragment:
        return shaderc_glsl_fragment_shader;
    case ShaderStage::Compute:
        return shaderc_glsl_compute_shader;
    }
    PICO_ASSERT_FAIL("Unknown shader stage: {}", static_cast<int>(stage));
}

ShaderStage StageFromExtension(std::string_view virtualPath)
{
    const size_t dot = virtualPath.rfind('.');
    PICO_ASSERT(dot != std::string_view::npos, "Shader path has no extension: '{}'", virtualPath);
    const std::string_view extension = virtualPath.substr(dot);
    if (extension == ".vert") return ShaderStage::Vertex;
    if (extension == ".frag" || extension == ".fsh") return ShaderStage::Fragment;
    if (extension == ".comp") return ShaderStage::Compute;
    PICO_ASSERT_FAIL("Cannot deduce shader stage from extension: '{}'", virtualPath);
}

std::string VirtualParent(std::string_view virtualPath)
{
    const size_t slash = virtualPath.rfind('/');
    return slash == std::string_view::npos ? std::string{} : std::string(virtualPath.substr(0, slash));
}

std::string_view VirtualFileName(std::string_view virtualPath)
{
    const size_t slash = virtualPath.rfind('/');
    return slash == std::string_view::npos ? virtualPath : virtualPath.substr(slash + 1);
}

bool JoinVirtual(std::string_view baseDir, std::string_view segment, std::string& out)
{
    std::vector<std::string_view> parts;
    if (!baseDir.empty())
    {
        for (size_t start = 0; start < baseDir.size();)
        {
            const size_t end = baseDir.find('/', start);
            parts.push_back(baseDir.substr(start, end == std::string_view::npos ? end : end - start));
            if (end == std::string_view::npos) break;
            start = end + 1;
        }
    }
    for (size_t start = 0; start < segment.size();)
    {
        const size_t           end  = segment.find('/', start);
        const std::string_view part = segment.substr(start, end == std::string_view::npos ? end : end - start);
        if (part.empty() || part == ".")
        { /* skip */
        }
        else if (part == "..")
        {
            if (parts.empty()) return false;
            parts.pop_back();
        }
        else
        {
            parts.push_back(part);
        }
        if (end == std::string_view::npos) break;
        start = end + 1;
    }
    out.clear();
    for (size_t i = 0; i < parts.size(); ++i)
    {
        if (i > 0) out += '/';
        out += parts[i];
    }
    return true;
}

struct IncludeRecord
{
    std::string name;    // virtual path; empty signals a failed inclusion
    std::string content; // file text, or the error message on failure
};

class VfsIncluder final : public shaderc::CompileOptions::IncluderInterface
{
  public:
    VfsIncluder(const IO::Vfs& vfs, std::vector<std::string> searchPrefixes,
                std::vector<std::string>* dependencies)
        : m_vfs(vfs), m_searchPrefixes(std::move(searchPrefixes)), m_dependencies(dependencies)
    {
    }

    shaderc_include_result* GetInclude(const char* requestedSource, shaderc_include_type type,
                                       const char* requestingSource, size_t /*includeDepth*/) override
    {
        auto* record      = new IncludeRecord();
        auto* result      = new shaderc_include_result{};
        result->user_data = record;

        if (Read(requestedSource, type, requestingSource, *record))
        {
            result->source_name        = record->name.c_str();
            result->source_name_length = record->name.size();
            result->content            = record->content.c_str();
            result->content_length     = record->content.size();
        }
        else
        {
            record->content            = std::format("Cannot resolve #include '{}'", requestedSource);
            result->source_name        = record->name.c_str(); // empty: signals failure to shaderc
            result->source_name_length = 0;
            result->content            = record->content.c_str();
            result->content_length     = record->content.size();
        }
        return result;
    }

    void ReleaseInclude(shaderc_include_result* data) override
    {
        delete static_cast<IncludeRecord*>(data->user_data);
        delete data;
    }

  private:
    bool Read(const char* requestedSource, shaderc_include_type type, const char* requestingSource,
              IncludeRecord& record)
    {
        const std::string_view requested(requestedSource != nullptr ? requestedSource : "");
        if (requested.empty()) return false;
        
        std::vector<std::string> candidates;
        const auto               push = [&candidates](std::string candidate) {
            if (!candidate.empty() && std::ranges::find(candidates, candidate) == candidates.end())
                candidates.push_back(std::move(candidate));
        };
        const bool             hasSlash   = requested.find('/') != std::string_view::npos;
        const bool             isRelative = type == shaderc_include_type_relative;
        const std::string_view requester(requestingSource != nullptr ? requestingSource : "");
        if (hasSlash) push(std::string(requested));
        if (isRelative && !requester.empty())
        {
            std::string joined;
            if (JoinVirtual(VirtualParent(requester), requested, joined)) push(joined);
        }
        if (!hasSlash)
            for (const auto& prefix : m_searchPrefixes)
                push(prefix + '/' + std::string(requested));
        for (const auto& candidate : candidates)
        {
            if (!m_vfs.Exists(candidate)) continue;
            record.content = m_vfs.ReadText(candidate);
            record.name    = candidate;
            if (m_dependencies != nullptr) m_dependencies->push_back(candidate);
            return true;
        }
        return false;
    }

    const IO::Vfs&            m_vfs;
    std::vector<std::string>  m_searchPrefixes;
    std::vector<std::string>* m_dependencies;
};
} // namespace

ShaderCompiler::ShaderCompiler(const IO::Vfs& vfs) : m_vfs(vfs)
{
}

void ShaderCompiler::AddIncludePrefix(std::string prefix)
{
    PICO_ASSERT(!prefix.empty() && prefix.back() != '/', "Include prefix must be a bare VFS directory: '{}'", prefix);
    m_includePrefixes.push_back(std::move(prefix));
}

CompiledShader ShaderCompiler::CompileFile(std::string_view virtualPath)
{
    return CompileFile(virtualPath, StageFromExtension(virtualPath));
}

CompiledShader ShaderCompiler::CompileFile(std::string_view virtualPath, ShaderStage stage)
{
    const std::string path(virtualPath);
    PICO_ASSERT(m_vfs.Exists(path), "Shader not found in VFS: '{}'", path);
    return CompileSource(path, m_vfs.ReadText(path), stage);
}

CompiledShader ShaderCompiler::CompileSource(std::string_view debugName, std::string_view source, ShaderStage stage,
                                             std::string_view includeDir)
{
    const std::string name(debugName);
    PICO_ASSERT(!source.empty(), "Empty shader source: {}", name);

    CompiledShader output;
    output.dependencies.push_back(name);

    std::vector<std::string> searchPrefixes = m_includePrefixes;
    const std::string        sourceDir      = VirtualParent(name);
    if (!sourceDir.empty()) searchPrefixes.insert(searchPrefixes.begin(), sourceDir);
    if (!includeDir.empty() && std::string(includeDir) != sourceDir)
        searchPrefixes.insert(searchPrefixes.begin(), std::string(includeDir));

    shaderc::CompileOptions options;
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
    options.SetWarningsAsErrors();
#ifdef NDEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
    options.SetGenerateDebugInfo();
#endif
    options.SetIncluder(std::make_unique<VfsIncluder>(m_vfs, std::move(searchPrefixes), &output.dependencies));

    const shaderc::SpvCompilationResult result =
        m_compiler.CompileGlslToSpv(source.data(), source.size(), ToKind(stage), name.c_str(), kEntryPoint, options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        LOG_ERROR("Shader compile failed '{}':\n{}", name, result.GetErrorMessage());
        PICO_ASSERT_FAIL("Shader compile failed: {}", name);
    }
    if (result.GetNumWarnings() > 0) LOG_WARNING("Shader warnings '{}':\n{}", name, result.GetErrorMessage());

    output.spirv.assign(result.cbegin(), result.cend());
    PICO_ASSERT(!output.spirv.empty(), "Empty SPIR-V output: {}", name);
    LOG_DEBUG("Compiled shader '{}' ({} bytes GLSL -> {} words SPIR-V)", name, source.size(), output.spirv.size());
    return output;
}
} // namespace PicoEngine::Rendering
