# rpg-sfml

Starter SFML 3 project for a 2D RPG, currently scoped to a single executable with a `Game`-centered runtime that opens a window and owns the main loop.

## Layout

```text
extlib/
  include/
    SFML/
  libs-<platform>-<compiler>/
    <arch>/
      libsfml-audio.so*
      libsfml-graphics.so*
      libsfml-network.so*
      libsfml-system.so*
      libsfml-window.so*
```

The project links those shared objects explicitly in CMake. It does not use SFML's packaged CMake metadata and does not keep static `*.a` libraries.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/src/main/main
```

The executable is configured with a build RPATH that points at the detected vendored SFML library directory, so it should find the shared libraries without extra environment variables.

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Structure

- Root `CMakeLists.txt`: only project-wide setup and subdirectories
- `src/main/CMakeLists.txt`: executable definition
- `include/main/Game.hpp`: public runtime interface for the `rpg::Game` entry object
- `src/main/Game.cpp`: SFML-backed runtime implementation and game loop phases
- `src/main/GameRuntimeSupport.hpp`: small runtime helpers shared by the implementation and tests
- `src/main/main.cpp`: minimal startup that constructs `rpg::Game` and calls `run()`
- `tests/`: CTest-based checks for runtime handoff and loop helper behavior
- `cmake/VendoredSFML.cmake`: vendored SFML detection and imported targets

## Vendored SFML selection

The helper uses plain CMake `if()` checks.

Right now the only built-in case is:

- Linux + GCC + x86_64 -> `extlib/libs-linux-gcc/x64`

Both vendored-path resolution and imported-target creation use explicit `if()` cases. Every other platform/compiler/architecture combination fails immediately with a clear CMake error instead of guessing and trying to load the wrong library type.

If you want to point at another vendored package explicitly, configure CMake with:

```bash
cmake -S . -B build -DRPG_SFML_LIB_DIR=/custom/path -DRPG_SFML_INCLUDE_DIR=/custom/include/path
```

To extend support later, add another explicit `if()` case in `cmake/VendoredSFML.cmake` for the new platform/compiler/architecture and point it at the matching `extlib/libs-.../...` directory.
