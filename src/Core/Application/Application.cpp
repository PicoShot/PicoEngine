#include "Application.hpp"
#include "Core/Debug/Debug.hpp"
#include "Core/Engine/Engine.hpp"
#include "Core/Window/Window.hpp"

namespace PicoEngine
{

int Application::Run()
{
    try
    {
        Engine engine;
        engine.Initialize();

        Window& window = engine.GetWindow();
        while (!window.ShouldClose())
            window.PollEvents();

        engine.Shutdown();
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR("Fatal: {}", ex.what());
        return 1;
    }
    return 0;
}

} // namespace PicoEngine
