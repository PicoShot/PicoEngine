#include "Application.hpp"
#include "Core/Window/Window.hpp"

#include <iostream>

namespace PicoEngine
{

int Application::Run()
{
    try
    {
        Window window(WindowDesc{});

        while (!window.ShouldClose())
        {
            window.PollEvents();
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[PicoEngine] Fatal: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}

} // namespace PicoEngine
