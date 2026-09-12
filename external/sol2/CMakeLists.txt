FetchContent_Declare(sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG e24392e9718f0616cfbba86005622a419d2f0c5d # v3.5.0
    GIT_PROGRESS TRUE
    SOURCE_SUBDIR pico-header-only)
# Header-only integration: do not let upstream search for a separate system Lua.
FetchContent_MakeAvailable(sol2)
add_library(PicoSol2 INTERFACE)
add_library(sol2::sol2 ALIAS PicoSol2)
target_include_directories(PicoSol2 SYSTEM INTERFACE "${sol2_SOURCE_DIR}/include")
target_compile_definitions(PicoSol2 INTERFACE SOL_LUAJIT=1 SOL_ALL_SAFETIES_ON=1)
target_link_libraries(PicoSol2 INTERFACE LuaJIT::LuaJIT)
