## Why

The project currently has the runtime shell and gameplay module boundaries in place, but it does not yet present a playable world slice. A deterministic procedural overworld slice gives the codebase a concrete gameplay target that exercises `World`, `Player`, `Camera`, and rendering together without expanding scope into combat, inventory, UI, or streaming systems.

## What Changes

- Add a deterministic overworld slice that generates the same tile layout from a fixed seed every run.
- Introduce basic tile-based overworld rendering for the generated map using simple built-in visuals rather than external art pipelines.
- Add player spawning and movement within the overworld slice using the existing `Player` module boundary.
- Add camera framing that tracks the player through the overworld slice using the existing `Camera` module boundary.
- Keep the slice intentionally narrow: world generation, player movement, camera follow behavior, and tile rendering are in scope; other gameplay systems remain out of scope.

## Capabilities

### New Capabilities
- `overworld-vertical-slice`: Defines deterministic procedural overworld generation, player placement and movement, camera tracking, and basic tile rendering for a playable vertical slice.

### Modified Capabilities

## Impact

- Affected specs: adds a new `overworld-vertical-slice` capability.
- Affected code: `src/main/Game.cpp`, `include/main/World.hpp`, `src/main/World.cpp`, `include/main/Player.hpp`, `src/main/Player.cpp`, `include/main/Camera.hpp`, `src/main/Camera.cpp`, and related tests under `tests/`.
- Runtime impact: the main loop will progress from an empty window to a simple playable overworld scene.
- Dependencies: continues to use the existing vendored SFML setup; no new external runtime dependencies are required.
