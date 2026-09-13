#pragma once

#include "Debug/Debug.hpp"
#include "IO/Vfs.hpp"

namespace PicoEngine
{

class Window;
namespace Rendering { class Renderer; }

class Engine
{
  public:
    Engine();
    ~Engine();

    Engine(const Engine&)            = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&)                 = delete;
    Engine& operator=(Engine&&)      = delete;

    static Engine& Get()
    {
        PICO_ASSERT(s_instance != nullptr, "Engine accessed before creation");
        return *s_instance;
    }

    void Initialize();
    void Shutdown();
    bool IsInitialized() const
    {
        return m_initialized;
    }

    Window&             GetWindow();
    Rendering::Renderer& GetRenderer();
    IO::Vfs&            GetVfs();

  private:
    static Engine* s_instance;

    bool m_initialized = false;

    IO::Vfs                          m_vfs;
    std::unique_ptr<Window>          m_window;
    std::unique_ptr<Rendering::Renderer> m_renderer;
};

} // namespace PicoEngine
