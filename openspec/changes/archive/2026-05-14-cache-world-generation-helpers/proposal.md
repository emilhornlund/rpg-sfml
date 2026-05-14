## Why

`World::ensureChunkRetained()` currently reconstructs the terrain and content generation helpers every time it loads a missing chunk. That extra setup work sits on the hot path for exploration and streaming, so tightening the world-owned chunk cache to retain those collaborators is a small refactor with a worthwhile runtime benefit.

## What Changes

- Update the world-owned chunk caching behavior so `World` retains its deterministic terrain and content generation collaborators instead of recreating them on each chunk miss.
- Preserve the current deterministic chunk tiles, metadata, and content results while moving helper construction to world-owned state initialized once per world instance.
- Keep chunk eviction behavior unchanged so unloading retained chunk data still allows deterministic regeneration when the chunk is requested again later.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `world-chunk-caching`: Tighten the chunk-retention contract so missing chunks reuse world-owned deterministic generation collaborators across loads within the same `World` instance.

## Impact

- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, and any world-generation helper wiring needed to preserve the current header boundary.
- Affected tests: world chunk caching and generation tests that cover chunk reuse and deterministic regeneration.
- No public API, dependency, or asset format changes.
