## Why

`Game.cpp` currently does more than runtime orchestration: it decides how the overworld session is initialized, translates live input into player intent, advances gameplay modules, and derives render-facing data directly from those modules. That makes the SFML runtime shell the place where overworld rules will naturally accumulate just as the project is preparing for larger procedural-world systems.

## What Changes

- Extract an overworld-specific runtime coordinator that owns `World`, `Player`, and `Camera` together as a gameplay session outside the top-level `Game` shell.
- Narrow `Game` to SFML-facing responsibilities such as window lifecycle, event polling, and drawing the current frame from gameplay-provided state.
- Introduce explicit boundaries for overworld input and render-facing state so gameplay rules stop depending directly on SFML input polling and ad hoc render assembly in `Game.cpp`.
- Preserve the existing procedural overworld behavior while moving initialization, update sequencing, and camera-follow orchestration behind the new coordinator.

## Capabilities

### New Capabilities

### Modified Capabilities

- `game-runtime`: Refine the runtime contract so `Game` acts as a thin executable shell around a dedicated overworld coordinator instead of directly owning gameplay orchestration details.
- `overworld-vertical-slice`: Refine the overworld slice contract so player spawn, frame-to-frame simulation, camera tracking, and render-facing overworld state are produced through the overworld coordinator boundary rather than assembled directly in `Game.cpp`.

## Impact

- Affected code: `src/main/Game.cpp`, `include/main/Game.hpp`, `src/main/`, `include/main/`, and related tests/build wiring.
- Affected design: runtime ownership, overworld session orchestration, input boundaries, and render-facing data flow.
- No external dependency changes are expected.
