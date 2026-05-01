## Why

`World` currently owns both the runtime chunk cache and the terrain-generation policy used to fill that cache. Splitting those concerns now keeps `World` focused on runtime world state before upcoming work on props, NPC spawns, points of interest, and save/load adds more generation-driven systems.

## What Changes

- Separate deterministic biome sampling and terrain-generation policy from `World`'s runtime cache and query responsibilities.
- Keep `World` as the gameplay-facing owner of retained chunk state, world-facing tile queries, and cache reuse across runtime reads and rendering.
- Introduce dedicated internal world-generation collaborators that can derive chunk content and other generated world data without moving those rules into `World`.
- Preserve existing deterministic terrain outcomes and runtime-facing world behavior while clarifying ownership boundaries for future generated content systems.

## Capabilities

### New Capabilities
<!-- None. -->

### Modified Capabilities
- `gameplay-modules`: refine the world module boundary so generation policy lives in dedicated collaborators while `World` remains the gameplay-facing owner of runtime world state.
- `world-chunk-caching`: clarify that chunk retention belongs to `World` while missing-chunk content is produced through dedicated generation collaborators rather than `World` owning generation policy directly.

## Impact

- Affected specs: `openspec/specs/gameplay-modules/spec.md`, `openspec/specs/world-chunk-caching/spec.md`
- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, related internal generation helpers under `src/main/`, and corresponding tests/build wiring
- No intended public API changes for `Game`, `Player`, `Camera`, or existing `World` consumers
