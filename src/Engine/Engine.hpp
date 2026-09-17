#pragma once

#include "Debug/Debug.hpp"
#include "IO/Vfs.hpp"
#include "Time/Time.hpp"

namespace PicoEngine
{

class Window;
namespace Rendering
{
class Renderer;
}

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

    bool Tick();

    void Run(const std::function<void()>& onFrame);

    Window&              GetWindow();
    Rendering::Renderer& GetRenderer();
    IO::Vfs&             GetVfs();
    Time&                GetTime();

  private:
    static Engine* s_instance;

    bool m_initialized = false;

    Time                                 m_time;
    IO::Vfs                              m_vfs;
    std::unique_ptr<Window>              m_window;
    std::unique_ptr<Rendering::Renderer> m_renderer;
};

} // namespace PicoEngine
