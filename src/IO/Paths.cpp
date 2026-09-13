#include "Paths.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine::IO
{
std::filesystem::path ExecutableDir()
{
    const char* base = SDL_GetBasePath();
    PICO_ASSERT(base != nullptr, "SDL_GetBasePath failed: {}", SDL_GetError());
    return std::filesystem::path(base);
}
} // namespace PicoEngine::IO
