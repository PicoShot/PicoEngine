#include "Debug.hpp"

namespace PicoEngine
{

void Debug::Assert(bool condition, std::string_view message, std::source_location location)
{
    if (condition)
        return;
    Write(Level::Error, std::format("ASSERT FAILED [{}:{}] {}", location.file_name(),
                                    location.line(), message));
    std::abort();
}

void Debug::Assert(bool condition, std::source_location location, std::string_view message)
{
    Assert(condition, message, location);
}

void Debug::Write(Level level, const std::string& message)
{
    const char* prefix = "?";
    switch (level)
    {
    case Level::Debug:
        prefix = "DEBUG";
        break;
    case Level::Warning:
        prefix = "WARN";
        break;
    case Level::Error:
        prefix = "ERROR";
        break;
    }

    std::cerr << '[' << prefix << "] " << message << '\n';
}

} // namespace PicoEngine
