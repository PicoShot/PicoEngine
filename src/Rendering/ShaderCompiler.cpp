#include "ShaderCompiler.hpp"
#include "Debug/Debug.hpp"

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

ShaderStage StageFromExtension(const std::filesystem::path& path)
{
    const auto extension = path.extension().string();
    if (extension == ".vert") return ShaderStage::Vertex;
    if (extension == ".frag" || extension == ".fsh") return ShaderStage::Fragment;
    if (extension == ".comp") return ShaderStage::Compute;
    PICO_ASSERT_FAIL("Cannot deduce shader stage from extension: {}", path.string());
}

struct IncludeRecord
{
    std::string name;    // absolute path; empty signals a failed inclusion
    std::string content; // file text, or the error message on failure
};

class FileIncluder final : public shaderc::CompileOptions::IncluderInterface
{
  public:
    FileIncluder(std::vector<std::filesystem::path> searchDirs, std::vector<std::filesystem::path>* dependencies)
        : m_searchDirs(std::move(searchDirs)), m_dependencies(dependencies)
    {
    }

    shaderc_include_result* GetInclude(const char* requestedSource, shaderc_include_type type,
                                       const char* requestingSource, size_t /*includeDepth*/) override
    {
        auto* record      = new IncludeRecord();
        auto* result      = new shaderc_include_result{};
        result->user_data = record;

        const std::filesystem::path requested(requestedSource);
        if (Read(requested, type, requestingSource, *record))
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
    bool Read(const std::filesystem::path& requested, shaderc_include_type type, const char* requestingSource,
              IncludeRecord& record)
    {
        std::vector<std::filesystem::path> candidates;
        if (requested.is_absolute())
        {
            candidates.push_back(requested);
        }
        else
        {
            if (type == shaderc_include_type_relative && requestingSource != nullptr && requestingSource[0] != '\0')
                candidates.push_back(std::filesystem::path(requestingSource).parent_path() / requested);
            for (const auto& dir : m_searchDirs)
                candidates.push_back(dir / requested);
        }
        for (const auto& candidate : candidates)
        {
            std::error_code statusError;
            if (!std::filesystem::is_regular_file(candidate, statusError)) continue;
            std::ifstream file(candidate, std::ios::binary);
            if (!file) continue;
            record.content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            if (file.bad()) continue;
            std::error_code absoluteError;
            record.name = std::filesystem::absolute(candidate, absoluteError).string();
            if (absoluteError) record.name = candidate.string();
            if (m_dependencies != nullptr) m_dependencies->push_back(std::filesystem::path(record.name));
            return true;
        }
        return false;
    }

    std::vector<std::filesystem::path>  m_searchDirs;
    std::vector<std::filesystem::path>* m_dependencies;
};
} // namespace

ShaderCompiler::ShaderCompiler() = default;

void ShaderCompiler::AddIncludeDir(std::filesystem::path dir)
{
    m_includeDirs.push_back(std::move(dir));
}

CompiledShader ShaderCompiler::CompileFile(const std::filesystem::path& path)
{
    return CompileFile(path, StageFromExtension(path));
}

CompiledShader ShaderCompiler::CompileFile(const std::filesystem::path& path, ShaderStage stage)
{
    std::ifstream file(path, std::ios::binary);
    PICO_ASSERT(file, "Cannot open shader: {}", path.string());
    std::string source{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{}};
    PICO_ASSERT(!file.bad(), "Cannot read shader: {}", path.string());
    std::error_code       absoluteError;
    std::filesystem::path absolute = std::filesystem::absolute(path, absoluteError);
    if (absoluteError) absolute = path;
    return CompileSource(absolute.string(), source, stage, absolute.parent_path());
}

CompiledShader ShaderCompiler::CompileSource(std::string_view debugName, std::string_view source, ShaderStage stage,
                                             const std::filesystem::path& includeDir)
{
    const std::string name(debugName);
    PICO_ASSERT(!source.empty(), "Empty shader source: {}", name);

    CompiledShader output;
    if (!includeDir.empty()) output.dependencies.push_back(includeDir / std::filesystem::path(name).filename());

    std::vector<std::filesystem::path> searchDirs = m_includeDirs;
    if (!includeDir.empty()) searchDirs.insert(searchDirs.begin(), includeDir);

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
    options.SetIncluder(std::make_unique<FileIncluder>(std::move(searchDirs), &output.dependencies));

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
