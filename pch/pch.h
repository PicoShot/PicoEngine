#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define PICO_PLATFORM_WINDOWS
#elif defined(__linux__)
#define PICO_PLATFORM_LINUX
#endif

#ifdef PICO_PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif

// STD
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// SDL3
#include <SDL3/SDL.h>