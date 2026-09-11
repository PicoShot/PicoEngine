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

find_package(Vulkan REQUIRED)

target_link_libraries(PicoEngine PRIVATE 
                      SDL3::SDL3-static
                      Vulkan::Vulkan)