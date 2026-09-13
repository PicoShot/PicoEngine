#pragma once

namespace PicoEngine::IO
{
class Backend
{
  public:
    virtual ~Backend()                                                        = default;
    virtual std::vector<std::byte> ReadBytes(const std::string& relativePath) = 0;
    virtual bool                   Exists(const std::string& relativePath)    = 0;
    virtual const char*            Name() const                               = 0;
    virtual std::string            Location() const                           = 0;
};
} // namespace PicoEngine::IO
