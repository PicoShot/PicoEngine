#pragma once
#include "Backend.hpp"

namespace PicoEngine::IO
{
class Vfs
{
  public:
    Vfs()                      = default;
    ~Vfs()                     = default;
    Vfs(const Vfs&)            = delete;
    Vfs& operator=(const Vfs&) = delete;

    void Mount(std::string prefix, std::unique_ptr<Backend> backend);
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
