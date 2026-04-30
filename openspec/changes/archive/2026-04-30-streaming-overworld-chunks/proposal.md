## Why

The current overworld is architected around a finite rectangular slice, so terrain generation intentionally produces water at the borders and the runtime stops serving terrain beyond the initial bounds. That blocks the next architectural goal: an overworld that can keep expanding around the player through deterministic chunk generation and retention.

## What Changes

- Replace the current finite-slice world boundary assumptions with a streaming overworld model that resolves terrain from absolute tile coordinates and loads chunks on demand.
- Update terrain-generation requirements so coastline and biome formation come from deterministic environmental signals rather than hard-coded border water or world-size edge falloff.
- Update world-facing rendering and traversal requirements so visible terrain queries ensure required chunks exist instead of clipping traversal to a preconfigured world rectangle.
- Update spawn and movement expectations so the runtime can start from a deterministic traversable anchor without depending on a finite map center.

## Capabilities

### New Capabilities
<!-- None. -->

### Modified Capabilities
- `overworld-vertical-slice`: Replace finite overworld slice assumptions with a streaming overworld that keeps deterministic terrain, spawn, movement, and camera behavior without a hard world edge.
- `visible-chunk-rendering`: Change visible terrain traversal from world-edge clipping to on-demand chunk-backed traversal for the camera-visible region.
- `world-chunk-caching`: Extend chunk retention requirements so missing chunks are generated on demand from absolute coordinates instead of only reusing a prepopulated finite cache.

## Impact

- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, `src/main/WorldTerrainGenerator.hpp`, `src/main/WorldTerrainGenerator.cpp`, and overworld-focused tests under `tests/`.
- Affected runtime behavior: terrain queries, visible tile enumeration, spawn selection, and the expectations around moving near former world edges.
- No new external dependencies are expected.
