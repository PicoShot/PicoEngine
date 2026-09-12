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
                      Vulkan::Vulkan
                      sol2::sol2
                      imgui::imgui
                      shaderc::shaderc
                      stb::stb)


# Optimizations
target_compile_options(PicoEngine PRIVATE -mavx2 -mfma)
set(PICO_OPTIMIZED_CONFIG "$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>")
target_compile_options(PicoEngine PRIVATE "$<${PICO_OPTIMIZED_CONFIG}:-O3>" "$<${PICO_OPTIMIZED_CONFIG}:-flto=thin>")
target_link_options(PicoEngine PRIVATE -fuse-ld=lld "$<${PICO_OPTIMIZED_CONFIG}:-flto=thin>")


# Assets
file(GLOB_RECURSE PICO_ASSET_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/assets/*")

add_custom_target(PicoAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:PicoEngine>/assets"
    DEPENDS ${PICO_ASSET_SOURCES}
    VERBATIM)

add_dependencies(PicoEngine PicoAssets)