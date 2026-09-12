#pragma once

#include "Debug/Debug.hpp"

namespace PicoEngine
{

class Window;

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

    Window& GetWindow();

  private:
    static Engine* s_instance;

    bool m_initialized = false;

    std::unique_ptr<Window> m_window;
};

} // namespace PicoEngine
