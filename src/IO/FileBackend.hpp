#pragma once
#include "Backend.hpp"

namespace PicoEngine::IO
{
class FileBackend final : public Backend
{
  public:
    explicit FileBackend(std::filesystem::path root);
    std::vector<std::byte> ReadBytes(const std::string& relativePath) override;
    bool                   Exists(const std::string& relativePath) override;
    const char*            Name() const override
    {
        return "file";
    }
    std::string Location() const override
    {
        return m_root.string();
    }

  private:
    std::filesystem::path m_root;
};
} // namespace PicoEngine::IO
