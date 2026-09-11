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

#include <iostream>
#include <string>
#include <vector>