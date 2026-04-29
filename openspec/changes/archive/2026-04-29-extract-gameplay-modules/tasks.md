## 1. Define gameplay module structure

- [x] 1.1 Add dedicated headers and source files for `World`, `Player`, and `Camera` in the runtime code layout.
- [x] 1.2 Declare the extracted gameplay module types in the project namespace with interfaces that keep room for future behavior growth.

## 2. Rewire the runtime coordinator

- [x] 2.1 Update `Game` internals to own and construct the extracted gameplay modules instead of defining placeholder structs in `Game.cpp`.
- [x] 2.2 Keep the main loop responsibilities in `Game` limited to orchestration, with gameplay type definitions and internal state living in the extracted modules.

## 3. Preserve build and verification coverage

- [x] 3.1 Update build configuration so the new runtime source files are compiled with the main executable.
- [x] 3.2 Add or adjust tests to cover the coordinator/module boundary without changing current runtime behavior.
