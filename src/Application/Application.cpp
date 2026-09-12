#include "Application.hpp"
#include "Engine/Engine.hpp"
#include "Window/Window.hpp"
#include "Samples/RotatingCube.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine
{

int Application::Run()
{

    try
    {
        PICO_ASSERT_FAIL("testing failed: {}", "test");
        Engine engine;
        engine.Initialize();
        Window&      window   = engine.GetWindow();
        auto&        renderer = engine.GetRenderer();
        RotatingCube cube(renderer);
        const auto   start  = std::chrono::steady_clock::now();
        uint64_t     frames = 0;
        while (!window.ShouldClose())
        {
            window.PollEvents();
            if (window.ShouldClose()) break;
            if (auto command = renderer.BeginFrame())
            {
                float seconds = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
                cube.Draw(command, seconds);
                renderer.EndFrame();
            }
        }
        return 0;
    }
    catch (const std::exception& exception)
    {
        LOG_ERROR("Application failed: {}", exception.what());
        return 1;
    }
}

} // namespace PicoEngine
