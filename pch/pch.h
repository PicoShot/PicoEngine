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

// GLM
#include <glm/glm.hpp>
#include <glm/simd/common.h>
#include <glm/simd/exponential.h>
#include <glm/simd/geometric.h>
#include <glm/simd/integer.h>
#include <glm/simd/matrix.h>
#include <glm/simd/packing.h>
#include <glm/simd/trigonometric.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>