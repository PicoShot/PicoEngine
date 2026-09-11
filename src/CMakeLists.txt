add_executable(PicoEngine)

file(GLOB_RECURSE PICO_SOURCES
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
    "*.hpp"
)

file(GLOB_RECURSE PICO_MODULES
    CONFIGURE_DEPENDS
    "*.cppm"
)

target_sources(PicoEngine
    PRIVATE
        ${PICO_SOURCES}
        ${CMAKE_SOURCE_DIR}/pch/pch.cpp
    PRIVATE
    FILE_SET CXX_MODULES FILES
        ${PICO_MODULES}
)

set_source_files_properties(${PICO_MODULES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
target_include_directories(PicoEngine PRIVATE ${CMAKE_SOURCE_DIR}/pch)
target_precompile_headers(PicoEngine PRIVATE ${CMAKE_SOURCE_DIR}/pch/pch.h)

find_package(Vulkan REQUIRED)

target_link_libraries(PicoEngine PRIVATE 
                      SDL3::SDL3-static
                      Vulkan::Vulkan)