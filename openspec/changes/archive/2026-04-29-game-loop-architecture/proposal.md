## Why

The project currently starts and runs entirely from a single `main.cpp` file that only opens an SFML window and processes close events. Before adding world generation, player movement, and RPG systems, the runtime needs a stable architecture that separates application startup, the game loop, and core game objects.

## What Changes

- Introduce a `Game`-centered runtime architecture that owns the SFML window and main loop.
- Define the responsibilities and boundaries for loop phases such as event processing, update, and render.
- Establish initial core runtime objects for the playable game flow, including game state coordination and extension points for world, player, and camera systems.
- Standardize the initial runtime file layout with the public `Game` declaration in `include/main/Game.hpp` and the implementation in `src/main/Game.cpp`.
- Establish runtime code conventions for project namespaces, file banners, include guards, and header hygiene.
- Require Doxygen documentation on the public runtime header so the runtime API is self-describing.
- Reduce `main.cpp` to startup and handoff into the runtime entry object.

## Capabilities

### New Capabilities
- `game-runtime`: Define the core runtime structure for starting the game, running the loop, and coordinating the main gameplay systems.

### Modified Capabilities

## Impact

- Affects `src/main/main.cpp` and the structure of the `src/main/` runtime code.
- Introduces `include/main/Game.hpp` and `src/main/Game.cpp` for the loop and top-level orchestration.
- Establishes the foundation for later features such as world simulation, procedural generation, rendering, and input handling.
- Constrains the initial implementation to keep SFML includes out of the public `Game` header when forward declarations and indirection are sufficient.
