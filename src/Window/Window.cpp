#include "Window.hpp"

#include "Debug/Debug.hpp"

namespace PicoEngine
{

namespace
{
int s_videoInitCount = 0;
} // namespace

Window::Window(const WindowDesc& desc) : m_width(desc.width), m_height(desc.height)
{
    const bool wasFirst = (s_videoInitCount == 0);
    if (wasFirst)
        PICO_ASSERT(SDL_Init(SDL_INIT_VIDEO), "SDL_Init failed: {}", SDL_GetError());

    SDL_WindowFlags flags = 0;

    flags |= SDL_WINDOW_VULKAN;

    if (desc.resizable)
        flags |= SDL_WINDOW_RESIZABLE;

    m_handle = SDL_CreateWindow(std::string(desc.title).c_str(), desc.width, desc.height, flags);
    PICO_ASSERT(m_handle != nullptr, "SDL_CreateWindow failed: {}", SDL_GetError());

    ++s_videoInitCount;
    LOG_DEBUG("Window created: '{}' {}x{}", desc.title, m_width, m_height);
}

Window::~Window()
{
    if (m_handle == nullptr) return;
    SDL_DestroyWindow(m_handle);
    m_handle = nullptr;

    --s_videoInitCount;
    if (s_videoInitCount == 0)
        SDL_Quit();
    LOG_DEBUG("Window destroyed");
}

Window::Window(Window&& other) noexcept
    : m_handle(other.m_handle), m_width(other.m_width), m_height(other.m_height),
      m_shouldClose(other.m_shouldClose)
{
    other.m_handle = nullptr;
}

Window& Window::operator=(Window&& other) noexcept
{
    if (this != &other)
    {
        if (m_handle != nullptr)
        {
            SDL_DestroyWindow(m_handle);
            --s_videoInitCount;
            if (s_videoInitCount == 0)
                SDL_Quit();
        }
        m_handle       = other.m_handle;
        m_width        = other.m_width;
        m_height       = other.m_height;
        m_shouldClose  = other.m_shouldClose;
        other.m_handle = nullptr;
    }
    return *this;
}

int32_t Window::GetWidth() const noexcept
{
    return m_width;
}

int32_t Window::GetHeight() const noexcept
{
    return m_height;
}

std::string Window::GetTitle() const noexcept
{
    PICO_ASSERT(m_handle == nullptr, "Window handle is null");
    return std::string(SDL_GetWindowTitle(m_handle));
}

void Window::SetTitle(std::string_view title)
{
    SDL_SetWindowTitle(m_handle, std::string(title).c_str());
}

void Window::PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            m_shouldClose = true;
            LOG_DEBUG("Quit requested");
            break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (event.window.windowID == SDL_GetWindowID(m_handle))
            {
                m_shouldClose = true;
                LOG_DEBUG("Window close requested");
            }
            break;

        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            if (event.window.windowID == SDL_GetWindowID(m_handle))
            {
                int w = 0, h = 0;
                SDL_GetWindowSize(m_handle, &w, &h);
                m_width  = static_cast<int32_t>(w);
                m_height = static_cast<int32_t>(h);
            }
            break;

        default:
            break;
        }
    }
}

} // namespace PicoEngine
