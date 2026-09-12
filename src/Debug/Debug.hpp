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

    static void ReportAssertion(std::source_location location, std::string_view expression,
                                std::string_view message = "Assertion failed") noexcept;

    template <typename... Args>
    static void ReportAssertion(std::source_location location, std::string_view expression,
                                std::format_string<Args...> fmt, Args&&... args) noexcept
    {
        try
        {
            ReportAssertion(location, expression, std::string_view(std::format(fmt, std::forward<Args>(args)...)));
        }
        catch (...)
        {
            ReportAssertion(location, expression, "Assertion message formatting failed");
        }
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

#define PICO_ASSERT(condition, ...)                                                                                       \
    do                                                                                                                    \
    {                                                                                                                     \
        if (!(condition)) [[unlikely]]                                                                                    \
        {                                                                                                                 \
            ::PicoEngine::Debug::ReportAssertion(std::source_location::current(), #condition __VA_OPT__(, ) __VA_ARGS__); \
            SDL_TriggerBreakpoint();                                                                                      \
            std::abort();                                                                                                 \
        }                                                                                                                 \
    } while (false)

#define PICO_ASSERT_FAIL(...)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        ::PicoEngine::Debug::ReportAssertion(std::source_location::current(), "unconditional failure" __VA_OPT__(, ) __VA_ARGS__); \
        SDL_TriggerBreakpoint();                                                                                                   \
        std::abort();                                                                                                              \
    } while (false)

#define LOG_DEBUG(...) ::PicoEngine::Debug::LogDebug(__VA_ARGS__)
#define LOG_WARNING(...) ::PicoEngine::Debug::LogWarning(__VA_ARGS__)
#define LOG_ERROR(...) ::PicoEngine::Debug::LogError(__VA_ARGS__)
