#include "Engine.hpp"
#include "IO/FileBackend/FileBackend.hpp"
#include "IO/Paths.hpp"
#include "Window/Window.hpp"
#include "Rendering/Renderer.hpp"

namespace PicoEngine
{

Engine* Engine::s_instance = nullptr;

Engine::Engine()
{
    PICO_ASSERT(s_instance == nullptr, "Only one Engine may exist");
    s_instance = this;
}

Engine::~Engine()
{
    Shutdown();
    s_instance = nullptr;
}

void Engine::Initialize()
{
    if (m_initialized)
        return;

    m_vfs.Mount<IO::FileBackend>("shaders", IO::ExecutableDir() / "assets" / "shaders");
    m_vfs.Mount<IO::FileBackend>("textures", IO::ExecutableDir() / "assets" / "textures");
    m_vfs.Mount<IO::FileBackend>("models", IO::ExecutableDir() / "assets" / "models");

    
    m_window    = std::make_unique<Window>(WindowDesc{});
    m_baseTitle = m_window->GetTitle();
    m_renderer  = std::make_unique<Rendering::Renderer>(m_window->GetHandle());
    m_time.Reset();
    m_initialized = true;
    m_renderer->SetVsync(false);
    LOG_DEBUG("Engine initialized");
}

void Engine::Shutdown()
{
    const bool wasInitialized = m_initialized;
    m_renderer.reset();
    m_window.reset();
    if (m_vfs.HasMount("models")) m_vfs.Unmount("models");
    if (m_vfs.HasMount("textures")) m_vfs.Unmount("textures");
    if (m_vfs.HasMount("shaders")) m_vfs.Unmount("shaders");
    m_initialized = false;
    if (wasInitialized) LOG_DEBUG("Engine shut down");
}

Window& Engine::GetWindow()
{
    PICO_ASSERT(m_window != nullptr, "Engine::GetWindow called before Initialize()");
    return *m_window;
}

Rendering::Renderer& Engine::GetRenderer()
{
    PICO_ASSERT(m_renderer != nullptr, "Renderer accessed before initialization");
    return *m_renderer;
}

IO::Vfs& Engine::GetVfs()
{
    PICO_ASSERT(m_initialized, "VFS accessed before Engine::Initialize()");
    return m_vfs;
}

Time& Engine::GetTime()
{
    PICO_ASSERT(m_initialized, "Time accessed before Engine::Initialize()");
    return m_time;
}

bool Engine::Tick()
{
    PICO_ASSERT(m_initialized, "Engine::Tick called before Initialize()");
    m_window->PollEvents();
    m_time.Update();
    UpdateTitleBar();
    return !m_window->ShouldClose();
}

void Engine::Run(const std::function<void()>& onFrame)
{
    PICO_ASSERT(m_initialized, "Engine::Run called before Initialize()");
    PICO_ASSERT(onFrame != nullptr, "Engine::Run requires a frame callback");
    while (Tick())
        onFrame();
}

void Engine::UpdateTitleBar()
{
    const float fps = m_time.GetFps();
    if (fps <= 0.0f)
        return;
    const int shown = static_cast<int>(fps + 0.5f);
    if (shown == m_lastShownFps)
        return;
    m_lastShownFps = shown;
    m_window->SetTitle(std::format("{} - {} FPS", m_baseTitle, shown));
}

} // namespace PicoEngine
