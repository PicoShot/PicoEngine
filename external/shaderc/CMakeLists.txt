FetchContent_Declare(shaderc
    GIT_REPOSITORY https://github.com/google/shaderc.git
    GIT_TAG 285bf6a2ab50b07bb1f777e7d9763169da93a34d
    SOURCE_SUBDIR pico-populate-only)
FetchContent_Declare(shaderc_glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG e1b562a8bed273a02f30b59b66a5d499793cede5
    SOURCE_SUBDIR pico-populate-only)
FetchContent_Declare(shaderc_spirv_headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG 04fd3caa1e8267e4d95c806cad901181728e1006
    SOURCE_SUBDIR pico-populate-only)
FetchContent_Declare(shaderc_spirv_tools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
    GIT_TAG ef96ed763b43b59b33b31b362f09a02b729fa1c9
    SOURCE_SUBDIR pico-populate-only)
FetchContent_MakeAvailable(shaderc shaderc_glslang shaderc_spirv_headers shaderc_spirv_tools)

set(SHADERC_GLSLANG_DIR "${shaderc_glslang_SOURCE_DIR}" CACHE STRING "" FORCE)
set(SHADERC_SPIRV_HEADERS_DIR "${shaderc_spirv_headers_SOURCE_DIR}" CACHE STRING "" FORCE)
set(SHADERC_SPIRV_TOOLS_DIR "${shaderc_spirv_tools_SOURCE_DIR}" CACHE STRING "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_WERROR_COMPILE OFF CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT OFF CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_SKIP_INSTALL_RULES ON)

add_subdirectory("${shaderc_SOURCE_DIR}" "${shaderc_BINARY_DIR}" EXCLUDE_FROM_ALL)
add_library(shaderc::shaderc ALIAS shaderc)
