<img src="https://github.com/emilhornlund/rpg-sfml/blob/main/.github/banner.png" alt="rpg-sfml banner" align="center" />

# rpg-sfml

## Overview

`rpg-sfml` is a small C++20/SFML project for a 2D RPG prototype. It currently builds a single executable with a `Game`-centered runtime that opens a window, runs the main loop, and coordinates gameplay-facing modules.

Key features:

- Single-executable desktop runtime built with CMake
- SFML-backed rendering and window loop
- Dedicated gameplay modules for `World`, `Player`, and `Camera`
- Deterministic overworld generation and chunk retention
- CTest-based coverage for runtime boundaries and gameplay helpers

## Tech Stack

- C++20
- CMake 3.24+
- SFML 3 (vendored headers and shared libraries)
- CTest

## Getting Started

### Prerequisites

- CMake 3.24 or newer
- A C++20-capable compiler
- Linux with GCC on x86_64 for the built-in vendored SFML configuration
- The required runtime assets available in `assets/`

The project does not use `find_package(SFML)` or packaged SFML CMake metadata. It links directly against vendored SFML shared libraries and expects the following layout under `extlib/`:

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

### Installation

Clone the repository:

```bash
git clone git@github.com:emilhornlund/rpg-sfml.git
cd rpg-sfml
```

The `assets/` directory is not bundled in this repository. It is configured as a Git submodule that points to a private repository:

```bash
git submodule update --init --recursive
```

If you do not have access to that private repository, the runtime assets will not be available after checkout. In that case, supply the required files in `assets/` before building or running the project.

If you want to use a different vendored SFML package, configure CMake with explicit include and library paths:

```bash
cmake -S . -B build -DRPG_SFML_LIB_DIR=/custom/path -DRPG_SFML_INCLUDE_DIR=/custom/include/path
```

### Build

```bash
cmake -S . -B build
cmake --build build
```

### Run

```bash
./build/src/main/main
```

The executable is configured with a build RPATH that points to the resolved vendored SFML library directory, so no extra environment variables are required for the default setup.

### Test

```bash
ctest --test-dir build --output-on-failure
```

## Project Structure

- `CMakeLists.txt` - project-wide CMake setup and subdirectory wiring
- `src/main/` - executable sources and runtime implementation
- `include/main/` - public headers for the main runtime modules
- `tests/` - CTest executables and boundary checks
- `cmake/VendoredSFML.cmake` - vendored SFML detection and imported target setup
- `assets/` - runtime asset directory expected by the build; sourced from a private submodule when available
- `extlib/` - vendored SFML headers and shared libraries

## Development Notes

- `Game` is the top-level runtime coordinator and public entry point.
- `World`, `Player`, and `Camera` hold gameplay-facing responsibilities and should remain separate from the top-level SFML runtime wiring.
- `include/main/Game.hpp` intentionally hides SFML-heavy runtime details behind a private implementation boundary.
- Runtime assets are staged from the repository-root `assets/` directory to `build/src/main/assets/` during the build.
- Built-in vendored SFML auto-detection currently supports only `Linux + GNU + x86_64`. Other platform, compiler, or architecture combinations fail with a clear CMake error until an explicit case is added in `cmake/VendoredSFML.cmake`.

## Assets / Licensing

- The code in this repository is released under the MIT License. See [`LICENSE`](LICENSE).
- Runtime assets are not bundled in this repository.
- The `assets/` directory is wired as a Git submodule that points to a private repository.
- Those assets are copied into the executable output as part of the build when they are available locally.
- If you change or redistribute asset content, review the asset repository separately and confirm its terms before publishing.

## Future Improvements

- Add explicit vendored SFML support for additional platforms and compilers
- Expand gameplay systems beyond the current overworld runtime slice
- Broaden test coverage as runtime and content pipelines grow
