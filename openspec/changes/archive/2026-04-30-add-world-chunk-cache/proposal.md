## Why

The current `World` implementation eagerly generates and stores a single finite tile grid, which couples terrain generation to fixed world dimensions and makes it hard to grow toward a larger procedural world. We need a first incremental step that introduces chunk-addressed generation and caching behind `World` so future world expansion can build on a stable runtime boundary instead of forcing a large rewrite later.

## What Changes

- Refactor world-owned terrain storage from one eagerly generated tile array to chunk-addressed generated data retained behind `World`.
- Add deterministic chunk coordinate handling so world tile queries can resolve tiles through chunk-local generation rather than a single prebuilt grid.
- Preserve the current gameplay-facing `World` role for traversability, tile lookup, spawn access, and coordinate conversion while moving chunk generation and caching details behind world-owned collaborators.
- Keep the current vertical-slice runtime behavior working during this step, including deterministic terrain queries and world-facing gameplay access through `World`.

## Capabilities

### New Capabilities
- `world-chunk-caching`: Define deterministic chunk-addressed terrain generation and cache behavior owned behind `World`.

### Modified Capabilities
- `gameplay-modules`: Extend the world module boundary so chunk generation and cache ownership evolve behind `World` rather than leaking into `Game` or other gameplay modules.
- `overworld-vertical-slice`: Update overworld generation requirements so the current slice remains deterministic when served through chunk-addressed world generation instead of a single eagerly generated grid.

## Impact

- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, `src/main/WorldTerrainGenerator.*`, runtime rendering and camera code that currently assume a fully eager finite world, and world-related tests.
- Affected behavior: world generation remains deterministic, but world-owned terrain lookup will be routed through chunk coordinate and cache logic.
- Affected design surface: `World` stays the public gameplay boundary while chunk generation and cache structures become explicit world-owned internals.
