#pragma once

struct SDL_Window;

namespace PicoEngine
{

struct WindowDesc
{
    std::string_view title     = "PicoEngine";
    int32_t          width     = 1280;
    int32_t          height    = 720;
    bool             resizable = true;
};

class Window
{
  public:
    explicit Window(const WindowDesc& desc = {});
    ~Window();

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    bool IsValid() const noexcept
    {
        return m_handle != nullptr;
    }

    SDL_Window* GetHandle() const noexcept
    {
        return m_handle;
    }

    int32_t     GetWidth() const noexcept;
    int32_t     GetHeight() const noexcept;
    std::string GetTitle() const noexcept;
    void        SetTitle(std::string_view title);
    void        PollEvents();

    bool ShouldClose() const noexcept
    {
        return m_shouldClose;
    }

    void RequestClose() noexcept
    {
        m_shouldClose = true;
    }

  private:
    SDL_Window* m_handle      = nullptr;
    int32_t     m_width       = 0;
    int32_t     m_height      = 0;
    bool        m_shouldClose = false;
};

} // namespace PicoEngine
