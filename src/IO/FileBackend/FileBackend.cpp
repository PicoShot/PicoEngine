#include "FileBackend.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine::IO
{
FileBackend::FileBackend(std::filesystem::path root)
{
    std::error_code error;
    m_root = std::filesystem::absolute(root, error);
    if (error) m_root = root;
    PICO_ASSERT(!m_root.empty(), "FileBackend requires a root folder");
    LOG_DEBUG("VFS backend '{}' rooted at '{}'", Name(), m_root.string());
}

std::vector<std::byte> FileBackend::ReadBytes(const std::string& relativePath)
{
    std::filesystem::path relative(relativePath, std::filesystem::path::format::generic_format);
    PICO_ASSERT(!relative.empty() && !relative.is_absolute(), "VFS path must be relative: '{}'", relativePath);
    for (const auto& part : relative)
        PICO_ASSERT(part != "..", "VFS path may not escape its backend: '{}'", relativePath);

    const std::filesystem::path full = (m_root / relative).lexically_normal();
    std::error_code             statusError;
    PICO_ASSERT(std::filesystem::is_regular_file(full, statusError), "VFS file not found: '{}' (backend '{}')",
                relativePath, Location());
    std::ifstream file(full, std::ios::binary | std::ios::ate);
    PICO_ASSERT(file, "VFS cannot open '{}' (backend '{}')", relativePath, Location());
    const auto size = file.tellg();
    PICO_ASSERT(size >= 0, "VFS cannot stat '{}' (backend '{}')", relativePath, Location());
    std::vector<std::byte> bytes(static_cast<size_t>(size));
    file.seekg(0);
    PICO_ASSERT(!bytes.empty() || size == 0, "VFS empty read '{}'", relativePath);
    if (!bytes.empty()) PICO_ASSERT(file.read(reinterpret_cast<char*>(bytes.data()), size), "VFS cannot read '{}'",
                                    relativePath);
    return bytes;
}

bool FileBackend::Exists(const std::string& relativePath)
{
    std::filesystem::path relative(relativePath, std::filesystem::path::format::generic_format);
    if (relative.empty() || relative.is_absolute()) return false;
    for (const auto& part : relative)
        if (part == "..") return false;
    std::error_code error;
    return std::filesystem::is_regular_file((m_root / relative).lexically_normal(), error);
}
} // namespace PicoEngine::IO
