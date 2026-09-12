#include "Engine.hpp"
#include "Window/Window.hpp"

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

    m_window      = std::make_unique<Window>(WindowDesc{});
    m_initialized = true;
    LOG_DEBUG("Engine initialized");
}

void Engine::Shutdown()
{
    if (!m_initialized)
        return;

    m_window.reset();
    m_initialized = false;
    LOG_DEBUG("Engine shut down");
}

Window& Engine::GetWindow()
{
    PICO_ASSERT(m_window != nullptr, "Engine::GetWindow called before Initialize()");
    return *m_window;
}

} // namespace PicoEngine
