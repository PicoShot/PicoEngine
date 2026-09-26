#include "Application.hpp"
#include "Debug/Debug.hpp"
#include "Engine/Engine.hpp"
#include "IO/Paths.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/SceneRenderer.hpp"

namespace PicoEngine
{

int Application::Run()
{
    try
    {
        Engine engine;
        engine.Initialize();
        auto& renderer = engine.GetRenderer();
        auto& scene    = engine.GetScene();
        auto& scripts  = engine.GetScripts();
        scripts.SetRenderContext(&renderer.GetDevice(), &engine.GetVfs());

        const std::filesystem::path scriptPath = IO::ExecutableDir() / "assets" / "scripts" / "main.lua";
        if (!scripts.ExecuteFile(scriptPath))
        {
            LOG_ERROR("Application failed: cannot run {}", scriptPath.string());
            return 1;
        }

        engine.Run([&]() {
            if (auto command = renderer.BeginFrame())
            {
                Rendering::SceneRenderer::Render(renderer, scene, command);
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
