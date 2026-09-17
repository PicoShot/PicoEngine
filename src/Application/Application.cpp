#include "Application.hpp"
#include "Engine/Engine.hpp"
#include "Samples/RotatingCube.hpp"
#include "Debug/Debug.hpp"

namespace PicoEngine
{

int Application::Run()
{

    try
    {
        Engine engine;
        engine.Initialize();
        auto&        renderer = engine.GetRenderer();
        RotatingCube cube(renderer, engine.GetVfs());
        engine.Run([&]() {
            if (auto command = renderer.BeginFrame())
            {
                cube.Draw(command, static_cast<float>(engine.GetTime().GetTime()));
                renderer.EndFrame();
            }
        });
        return 0;
    }
    catch (const std::exception& exception)
    {
        LOG_ERROR("Application failed: {}", exception.what());
        return 1;
    }
}

} // namespace PicoEngine
