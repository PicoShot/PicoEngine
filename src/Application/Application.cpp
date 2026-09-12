#include "Application.hpp"
#include "Engine/Engine.hpp"
#include "Window/Window.hpp"

namespace PicoEngine
{

int Application::Run()
{

    Engine engine;
    engine.Initialize();

    Window& window = engine.GetWindow();
    while (!window.ShouldClose())
        window.PollEvents();

    engine.Shutdown();

    return 0;
}

} // namespace PicoEngine
