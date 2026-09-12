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



# Optimizations

if (MSVC)
    target_compile_options(PicoEngine PRIVATE /arch:AVX2)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(PicoEngine PRIVATE -mavx2 -mfma)
endif()

set(PICO_OPTIMIZED_CONFIG "$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        target_compile_options(PicoEngine PRIVATE
            "$<${PICO_OPTIMIZED_CONFIG}:/clang:-O3>"
            "$<${PICO_OPTIMIZED_CONFIG}:/clang:-flto=thin>")
        target_link_options(PicoEngine PRIVATE "$<${PICO_OPTIMIZED_CONFIG}:-fuse-ld=lld>")
    else()
        target_compile_options(PicoEngine PRIVATE
            "$<${PICO_OPTIMIZED_CONFIG}:-O3>"
            "$<${PICO_OPTIMIZED_CONFIG}:-flto=thin>")
        target_link_options(PicoEngine PRIVATE
            "$<${PICO_OPTIMIZED_CONFIG}:-flto=thin>"
            "$<${PICO_OPTIMIZED_CONFIG}:-fuse-ld=lld>")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR MSVC)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT PICO_IPO_SUPPORTED OUTPUT PICO_IPO_ERROR LANGUAGES CXX)
    if (NOT PICO_IPO_SUPPORTED)
        message(FATAL_ERROR "Release LTO is unavailable: ${PICO_IPO_ERROR}")
    endif()
    set_target_properties(PicoEngine PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
        INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    if (MSVC)
        target_compile_options(PicoEngine PRIVATE "$<${PICO_OPTIMIZED_CONFIG}:/O2>")
    else()
        target_compile_options(PicoEngine PRIVATE "$<${PICO_OPTIMIZED_CONFIG}:-O3>")
    endif()
endif()
