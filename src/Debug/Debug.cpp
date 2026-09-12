#include "Debug.hpp"

namespace PicoEngine
{

void Debug::ReportAssertion(std::source_location location, std::string_view expression, std::string_view message) noexcept
{
    try
    {
        Write(Level::Error, std::format("ASSERT FAILED [{}:{}] ({}) {}", location.file_name(),
                                      location.line(), expression, message));
    }
    catch (...)
    {
        // Logging failure must not prevent the call-site breakpoint and abort.
    }
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
