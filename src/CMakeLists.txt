add_executable(PicoEngine)

file(GLOB_RECURSE PICO_SOURCES
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
    "*.hpp"
)

target_sources(PicoEngine PRIVATE
        ${PICO_SOURCES}
        ${CMAKE_SOURCE_DIR}/pch/pch.cpp
)

target_include_directories(PicoEngine PRIVATE ${CMAKE_SOURCE_DIR}/pch ${CMAKE_SOURCE_DIR}/src)
target_precompile_headers(PicoEngine PRIVATE ${CMAKE_SOURCE_DIR}/pch/pch.h)

target_link_libraries(PicoEngine PRIVATE 
                      SDL3::SDL3-static
                      glm::glm
                      Vulkan::Vulkan)

if (MSVC)
    target_compile_options(PicoEngine PRIVATE /arch:AVX2)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(PicoEngine PRIVATE -mavx2 -mfma)
endif()