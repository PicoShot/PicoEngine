#pragma once

namespace PicoEngine
{

class Debug
{
  public:
    Debug() = delete;

    template <typename... Args>
    static void LogDebug(std::format_string<Args...> fmt, Args&&... args)
    {
        Write(Level::Debug, std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void LogWarning(std::format_string<Args...> fmt, Args&&... args)
    {
        Write(Level::Warning, std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void LogError(std::format_string<Args...> fmt, Args&&... args)
    {
        Write(Level::Error, std::format(fmt, std::forward<Args>(args)...));
    }

    static void Assert(bool condition, std::string_view message = "Assertion failed",
                       std::source_location location = std::source_location::current());

    static void Assert(bool condition, std::source_location location,
                       std::string_view message = "Assertion failed");

    template <typename... Args>
    static void Assert(bool condition, std::source_location location,
                       std::format_string<Args...> fmt, Args&&... args)
    {
        if (!condition)
            Assert(false, location, std::format(fmt, std::forward<Args>(args)...));
    }

  private:
    enum class Level
    {
        Debug,
        Warning,
        Error
    };

    static void Write(Level level, const std::string& message);
};

} // namespace PicoEngine

#define PICO_ASSERT(condition, ...) \
    ::PicoEngine::Debug::Assert((condition), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)

#define LOG_DEBUG(...) ::PicoEngine::Debug::LogDebug(__VA_ARGS__)
#define LOG_WARNING(...) ::PicoEngine::Debug::LogWarning(__VA_ARGS__)
#define LOG_ERROR(...) ::PicoEngine::Debug::LogError(__VA_ARGS__)
