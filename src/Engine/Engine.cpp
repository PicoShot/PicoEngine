#include "Engine.hpp"
#include "IO/FileBackend.hpp"
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

    m_vfs.Mount("shaders", std::make_unique<IO::FileBackend>(IO::ExecutableDir() / "assets" / "shaders"));
    m_vfs.Mount("textures", std::make_unique<IO::FileBackend>(IO::ExecutableDir() / "assets" / "textures"));
    m_vfs.Mount("models", std::make_unique<IO::FileBackend>(IO::ExecutableDir() / "assets" / "models"));
    m_window      = std::make_unique<Window>(WindowDesc{});
    m_renderer    = std::make_unique<Rendering::Renderer>(m_window->GetHandle());
    m_initialized = true;
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

} // namespace PicoEngine
