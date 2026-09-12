FetchContent_Declare(luajit
    GIT_REPOSITORY https://github.com/LuaJIT/LuaJIT.git
    GIT_TAG c6ffc141a8762b41703f9287d63d93622a13dd8f
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(luajit)

if (CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR "The LuaJIT integration currently requires a native Windows or Linux build")
endif()

set(LUAJIT_BUILD_ROOT "${luajit_BINARY_DIR}/$<CONFIG>")
set(LUAJIT_INCLUDE_DIR "${luajit_BINARY_DIR}/include/$<CONFIG>")
if (WIN32)
    get_filename_component(PICO_LLVM_BIN "${CMAKE_C_COMPILER}" DIRECTORY)
    foreach(TOOL clang-cl lld-link llvm-lib)
        if (NOT EXISTS "${PICO_LLVM_BIN}/${TOOL}.exe")
            message(FATAL_ERROR "LuaJIT requires ${TOOL}.exe alongside clang.exe")
        endif()
    endforeach()
    find_program(PICO_VSWHERE vswhere PATHS "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer" REQUIRED)
    execute_process(COMMAND "${PICO_VSWHERE}" -latest -products *
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        OUTPUT_VARIABLE PICO_VS_PATH OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
    if (NOT EXISTS "${PICO_VS_PATH}/Common7/Tools/VsDevCmd.bat")
        message(FATAL_ERROR "LuaJIT requires Visual Studio C++ build tools")
    endif()
    if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8 OR CMAKE_SYSTEM_PROCESSOR MATCHES "ARM|arm")
        message(FATAL_ERROR "The Windows LuaJIT build currently supports x64")
    endif()
    set(LUAJIT_LIBRARY "${LUAJIT_BUILD_ROOT}/src/lua51.lib")
    file(GENERATE OUTPUT "${luajit_BINARY_DIR}/build-$<CONFIG>.bat" CONTENT
"@echo off
call \"${PICO_VS_PATH}/Common7/Tools/VsDevCmd.bat\" -arch=x64 -host_arch=x64 -no_logo
if errorlevel 1 exit /b 1
set CL=/MD$<$<CONFIG:Debug>:d>
call clangbuild.bat static
if not exist lua51.lib exit /b 1
")
    set(LUAJIT_BUILD_COMMAND cmd /c "${luajit_BINARY_DIR}/build-$<CONFIG>.bat")
    set(LUAJIT_PREPARE_COMMAND ${CMAKE_COMMAND}
        "-DBUILD_ROOT=${LUAJIT_BUILD_ROOT}" "-DLLVM_BIN=${PICO_LLVM_BIN}"
        -P "${CMAKE_CURRENT_LIST_DIR}/ClangBuild.cmake")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_program(PICO_MAKE NAMES gmake make REQUIRED)
    set(LUAJIT_LIBRARY "${LUAJIT_BUILD_ROOT}/src/libluajit.a")
    set(LUAJIT_BUILD_COMMAND "${PICO_MAKE}" -C "${LUAJIT_BUILD_ROOT}/src"
        "CC=${CMAKE_C_COMPILER}" BUILDMODE=static "XCFLAGS=-fPIC")
    set(LUAJIT_PREPARE_COMMAND ${CMAKE_COMMAND} -E true)
else()
    message(FATAL_ERROR "LuaJIT integration supports Windows and Linux")
endif()

add_custom_command(OUTPUT "${LUAJIT_LIBRARY}"
    BYPRODUCTS "${LUAJIT_INCLUDE_DIR}/lua.h" "${LUAJIT_INCLUDE_DIR}/lauxlib.h"
        "${LUAJIT_INCLUDE_DIR}/lualib.h" "${LUAJIT_INCLUDE_DIR}/luaconf.h"
        "${LUAJIT_INCLUDE_DIR}/luajit.h"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${LUAJIT_BUILD_ROOT}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LUAJIT_BUILD_ROOT}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${luajit_SOURCE_DIR}" "${LUAJIT_BUILD_ROOT}"
    COMMAND ${LUAJIT_PREPARE_COMMAND}
    COMMAND ${CMAKE_COMMAND} -E chdir "${LUAJIT_BUILD_ROOT}/src" ${LUAJIT_BUILD_COMMAND}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LUAJIT_INCLUDE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LUAJIT_BUILD_ROOT}/src/lua.h" "${LUAJIT_BUILD_ROOT}/src/lauxlib.h"
        "${LUAJIT_BUILD_ROOT}/src/lualib.h" "${LUAJIT_BUILD_ROOT}/src/luaconf.h"
        "${LUAJIT_BUILD_ROOT}/src/luajit.h" "${LUAJIT_INCLUDE_DIR}"
    DEPENDS "${CMAKE_CURRENT_LIST_FILE}" "${CMAKE_CURRENT_LIST_DIR}/ClangBuild.cmake"
        "${luajit_SOURCE_DIR}/src/msvcbuild.bat"
    COMMENT "Building static LuaJIT"
    VERBATIM)
add_custom_target(PicoLuaJITBuild DEPENDS "${LUAJIT_LIBRARY}")
add_library(PicoLuaJIT INTERFACE)
add_library(LuaJIT::LuaJIT ALIAS PicoLuaJIT)
add_dependencies(PicoLuaJIT PicoLuaJITBuild)
target_include_directories(PicoLuaJIT SYSTEM INTERFACE "${LUAJIT_INCLUDE_DIR}")
target_link_libraries(PicoLuaJIT INTERFACE "${LUAJIT_LIBRARY}")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(PicoLuaJIT INTERFACE ${CMAKE_DL_LIBS} m)
endif()
