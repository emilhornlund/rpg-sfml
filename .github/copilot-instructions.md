# Copilot instructions for rpg-sfml

This repository is a small C++20/CMake SFML project. Keep changes narrow, repo-native, and easy to review.

## Project shape

- The executable lives in `src/main` and is named `main`.
- Public headers live under `include/main/`; source files live under `src/main/`.
- Tests live under `tests/` and are wired through CTest.
- Keep the current module boundaries intact: `Game` is the top-level runtime coordinator, while `World`, `Player`, and `Camera` are the gameplay-facing modules.
- Prefer extending existing modules over introducing new architectural layers for small features.

## Build and test commands

Prefer the commands already documented in `README.md`:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## C++ and CMake conventions

- Target C++20 and keep code compatible with the current CMake setup.
- Keep the root `CMakeLists.txt` limited to project-wide setup and subdirectories.
- Put executable-specific wiring in `src/main/CMakeLists.txt` and test wiring in `tests/CMakeLists.txt`.
- Reuse the existing vendored SFML flow in `cmake/VendoredSFML.cmake`; do not switch the project to `find_package`, FetchContent, or a package manager unless the task explicitly requires it.
- Keep GNU/Clang builds warning-clean under the existing `-Wall -Wextra -Wpedantic` flags.

## Header and source style

- Follow `.editorconfig`: 4-space indentation, LF endings, UTF-8, final newline, max line length 120.
- Use include guards, not `#pragma once`.
- Keep the existing MIT file banner in C++ source and header files.
- Preserve the current namespace and brace style:
  - `namespace rpg` on its own line
  - opening braces on the next line
  - closing comments like `} // namespace rpg`
- Use `m_` prefixes for member variables.
- Public headers should use Doxygen-style comments when defining public API or important boundaries.
- Keep implementation comments sparse and only add them when they clarify non-obvious logic.

## Runtime boundaries

- `include/main/Game.hpp` intentionally hides SFML-heavy runtime details behind a private implementation. Preserve that boundary unless a task clearly requires changing the public API.
- Avoid pushing gameplay responsibilities into `Game` when they belong in `World`, `Player`, or `Camera`.
- If logic can be isolated away from SFML window code, prefer small helpers like `GameRuntimeSupport.hpp` so behavior stays easy to test.

## Testing expectations

- Add or update tests in `tests/` when behavior changes.
- Prefer small, focused CTest executables and lightweight assertions over introducing a new test framework.
- Keep tests aligned with the current style: straightforward `main()`-based checks, `static_assert` where appropriate, and explicit failures through return codes.

## Change scope

- Do not edit vendored libraries under `extlib/` unless the task is explicitly about vendored dependencies.
- Do not reorganize the directory structure without a clear requirement.
- Respect the existing `.github/prompts/` and `.github/skills/` OpenSpec workflow files; only change them when the task is specifically about that workflow.
- Prefer incremental, minimal changes that match the current code rather than speculative abstractions.
