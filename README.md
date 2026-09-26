# PicoEngine

A personal C++ game engine for experimenting with modern tech — primarily **Vulkan** and **SDL3** — with a Unity-inspired workflow: scenes, game objects, components, and hybrid Lua/C++ scripting.

## Features (in progress)

- Vulkan renderer (graphics pipeline, runtime shader compilation via shaderc)
- SDL3 windowing and input
- Scene / GameObject / Component model with C++ behaviours
- LuaJIT gameplay scripting via sol2 (C++-owned lifetime, Lua holds handles)

## Requirements

### Toolchain

- LLVM Clang
- CMake ≥ 4.1, Ninja, LLD (used for engine linking)

### System

- OS: Windows or Linux
- Vulkan SDK
- CPU with AVX2/FMA support

## Build

```sh
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```

## License

MIT — see [LICENSE](LICENSE).
