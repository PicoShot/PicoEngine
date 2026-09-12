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

set(PICO_SHADER_OUTPUTS)
foreach(SHADER cube.vert cube.frag)
    set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER}.spv")
    add_custom_command(OUTPUT "${OUTPUT}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders"
        COMMAND Vulkan::glslc "${CMAKE_CURRENT_SOURCE_DIR}/Samples/Shaders/${SHADER}" -o "${OUTPUT}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Samples/Shaders/${SHADER}"
        VERBATIM)
    list(APPEND PICO_SHADER_OUTPUTS "${OUTPUT}")
endforeach()
add_custom_target(PicoShaders ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PicoEngine>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PICO_SHADER_OUTPUTS} "$<TARGET_FILE_DIR:PicoEngine>/shaders"
    DEPENDS ${PICO_SHADER_OUTPUTS}
    VERBATIM)
add_dependencies(PicoEngine PicoShaders)

target_link_libraries(PicoEngine PRIVATE 
                      SDL3::SDL3-static
                      glm::glm
                      Vulkan::Vulkan)

if (MSVC)
    target_compile_options(PicoEngine PRIVATE /arch:AVX2)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(PicoEngine PRIVATE -mavx2 -mfma)
endif()
