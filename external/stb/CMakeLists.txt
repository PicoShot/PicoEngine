FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG 2c980bb59875b0d32144a71867fbdebb2f77cd20
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(stb)

add_library(PicoStb STATIC stb_image.c)
add_library(stb::stb ALIAS PicoStb)
target_include_directories(PicoStb SYSTEM PUBLIC "${stb_SOURCE_DIR}")
