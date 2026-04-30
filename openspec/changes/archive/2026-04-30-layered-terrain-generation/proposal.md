## Why

The current overworld terrain is deterministic but visually noisy because each tile is classified from an almost independent hash result. Reworking generation around layered elevation and moisture signals will make the map read as connected landforms and biomes instead of random tile scatter.

## What Changes

- Replace single-sample tile classification with deterministic layered terrain signals for elevation and moisture.
- Map combined elevation and moisture values to the existing tile categories so coastlines, inland grasslands, forests, and water emerge from spatially coherent rules.
- Preserve deterministic world generation, traversable spawn selection, and the existing world-facing runtime APIs while improving terrain continuity.
- Add tests that lock in the new terrain-generation guarantees and biome-mapping behavior.

## Capabilities

### New Capabilities
- None.

### Modified Capabilities
- `overworld-vertical-slice`: change overworld generation requirements so terrain emerges from layered environmental signals and tile types are assigned from those signals instead of per-tile random thresholds.

## Impact

- Affected code: `src/main/WorldTerrainGenerator.cpp`, `src/main/World.cpp`, terrain-related tests, and any rendering that depends on tile categories.
- APIs: no planned public API expansion for `World`; behavior changes are internal to generation rules.
- Dependencies: no new external dependencies planned; implementation should stay within the existing C++20/CMake setup.
