#include "Vfs.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine::IO
{
namespace
{
constexpr char kSeparator = '/';

bool Split(std::string_view virtualPath, std::string_view& prefix, std::string_view& relative)
{
    if (virtualPath.empty() || virtualPath.front() == kSeparator || virtualPath.back() == kSeparator) return false;
    const size_t slash = virtualPath.find(kSeparator);
    if (slash == std::string_view::npos) return false;
    if (virtualPath.find('\\') != std::string_view::npos) return false;
    prefix   = virtualPath.substr(0, slash);
    relative = virtualPath.substr(slash + 1);
    if (prefix.empty() || relative.empty() || relative.find("//") != std::string_view::npos) return false;
    for (size_t start = 0; start <= relative.size();)
    {
        const size_t end    = relative.find(kSeparator, start);
        const size_t length = (end == std::string_view::npos ? relative.size() : end) - start;
        if (length == 2 && relative[start] == '.' && relative[start + 1] == '.') return false;
        if (end == std::string_view::npos) break;
        start = end + 1;
    }
    return true;
}
} // namespace

void Vfs::Mount(std::string prefix, std::unique_ptr<Backend> backend)
{
    PICO_ASSERT(!prefix.empty() && prefix.find(kSeparator) == std::string::npos && prefix.find('\\') == std::string::npos,
                "VFS mount prefix must be a single path segment: '{}'", prefix);
    PICO_ASSERT(backend != nullptr, "Cannot mount '{}' with a null backend", prefix);
    for (const auto& mount : m_mounts)
        PICO_ASSERT(mount.prefix != prefix, "VFS prefix '{}' is already mounted", prefix);
    LOG_DEBUG("VFS mounted '{}' -> {} '{}'", prefix, backend->Name(), backend->Location());
    m_mounts.push_back(MountPoint{std::move(prefix), std::move(backend)});
}

void Vfs::Unmount(std::string_view prefix)
{
    const auto it = std::find_if(m_mounts.begin(), m_mounts.end(),
                                 [prefix](const MountPoint& mount) { return mount.prefix == prefix; });
    PICO_ASSERT(it != m_mounts.end(), "VFS cannot unmount unknown prefix '{}'", prefix);
    LOG_DEBUG("VFS unmounted '{}'", prefix);
    m_mounts.erase(it);
}

bool Vfs::HasMount(std::string_view prefix) const
{
    return std::ranges::any_of(m_mounts, [prefix](const MountPoint& mount) { return mount.prefix == prefix; });
}

std::vector<std::byte> Vfs::ReadBytes(std::string_view virtualPath) const
{
    std::string_view relative;
    const auto*      mount = Find(virtualPath, relative);
    PICO_ASSERT(mount != nullptr, "VFS has no mount for '{}'", virtualPath);
    return mount->backend->ReadBytes(std::string(relative));
}

std::string Vfs::ReadText(std::string_view virtualPath) const
{
    const auto bytes = ReadBytes(virtualPath);
    return {reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}

bool Vfs::Exists(std::string_view virtualPath) const
{
    std::string_view relative;
    const auto*      mount = Find(virtualPath, relative);
    return mount != nullptr && mount->backend->Exists(std::string(relative));
}

const Vfs::MountPoint* Vfs::Find(std::string_view virtualPath, std::string_view& relative) const
{
    std::string_view prefix;
    if (!Split(virtualPath, prefix, relative)) return nullptr;
    for (const auto& mount : m_mounts)
        if (mount.prefix == prefix) return &mount;
    return nullptr;
}
} // namespace PicoEngine::IO
