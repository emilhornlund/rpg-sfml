## Why

`src/main/Game.cpp` remains the main executable complexity sink even after earlier helper extractions. It still mixes resource bootstrap, SFML event translation, frame diagnostics, render-plan assembly, player-occlusion compositing, and debug overlay drawing in one file, which raises the cost and risk of future runtime changes.

The next refactor should reduce that concentration without changing gameplay behavior. Capturing the decomposition as one coordinated change keeps the tickets sequenced around current test constraints and avoids a series of unrelated proposals for tightly coupled runtime-shell work.

## What Changes

- Extract the debug overlay pipeline out of `Game.cpp` first, including shell-owned overlay state updates, frame diagnostics preparation, and screen-space overlay drawing orchestration.
- Extract runtime resource bootstrap concerns out of `Game.cpp`, including terrain, vegetation, player, font, and shader loading owned by the SFML shell.
- Extract SFML event translation out of `Game.cpp` so input polling and key-to-runtime action mapping no longer live inline with the frame loop coordinator.
- Unpin rendering boundary coverage from `Game.cpp`-literal checks so deeper shell refactors can move render-planning and occlusion code without weakening protection.
- Extract render-plan assembly from `Game.cpp`, including render-queue construction, stable ordering, player-marker lookup, front-occluder qualification, and debug render metrics preparation.
- Extract the player-occlusion compositor from `Game.cpp`, including reduced-resolution working-surface sizing, mask drawing, shader uniform setup, and final silhouette composition.
- Extract the overworld scene renderer from `Game.cpp` so the top-level game shell coordinates world rendering through dedicated support collaborators instead of issuing every pass inline.

## Capabilities

### New Capabilities
- `game-shell-decomposition`: Defines the executable-shell decomposition boundaries that move shell-owned runtime concerns out of `Game.cpp` while preserving the current public `Game` entry point and frame behavior.

### Modified Capabilities
- `game-runtime`: Clarify that the top-level `Game` shell should coordinate shell-owned runtime concerns through dedicated collaborators instead of keeping those concerns inline in `Game.cpp`.

## Impact

- Affected code: `src/main/Game.cpp`, `src/main/CMakeLists.txt`, new or expanded `src/main/*Support.*` runtime-shell helpers, and targeted tests under `tests/`.
- Affected systems: runtime bootstrap, input/event handling, frame diagnostics, render planning, player occlusion compositing, and overworld scene rendering.
- Public API impact: none expected for `include/main/Game.hpp`; this change is intended to preserve current runtime behavior while tightening internal boundaries.
