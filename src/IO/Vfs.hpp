#pragma once
#include "Backend.hpp"

namespace PicoEngine::IO
{
template <typename T>
concept VfsBackend = std::derived_from<T, Backend> && !std::is_abstract_v<T>;

class Vfs
{
  public:
    Vfs()                      = default;
    ~Vfs()                     = default;
    Vfs(const Vfs&)            = delete;
    Vfs& operator=(const Vfs&) = delete;

    void Mount(std::string prefix, std::unique_ptr<Backend> backend);

    template <VfsBackend T, typename... Args>
        requires std::constructible_from<T, Args...>
    void Mount(std::string prefix, Args&&... args)
    {
        Mount(std::move(prefix), std::make_unique<T>(std::forward<Args>(args)...));
    }

    void Unmount(std::string_view prefix);
    bool HasMount(std::string_view prefix) const;

    std::vector<std::byte> ReadBytes(std::string_view virtualPath) const;
    std::string            ReadText(std::string_view virtualPath) const;
    bool                   Exists(std::string_view virtualPath) const;

  private:
    struct MountPoint
    {
        std::string              prefix;
        std::unique_ptr<Backend> backend;
    };
    const MountPoint* Find(std::string_view virtualPath, std::string_view& relative) const;

    std::vector<MountPoint> m_mounts;
};

} // namespace PicoEngine::IO
