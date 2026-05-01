## Why

The overworld currently renders terrain by filling SFML rectangle shapes with hard-coded colors. That keeps the vertical slice simple, but it blocks the next step toward a tile-based presentation pipeline and leaves no asset-backed path for terrain visuals.

## What Changes

- Add an asset-backed overworld terrain rendering path that draws visible terrain tiles from a shared tileset texture instead of per-tile fill colors.
- Introduce a minimal first tileset containing the existing four terrain categories: grass, sand, water, and forest.
- Reduce the overworld runtime tile size from 32 world units to 16 so terrain geometry aligns with the intended future 16x16 tileset baseline.
- Define a stable mapping from overworld `TileType` values to tileset cells so the current gameplay-facing render snapshot can continue to drive drawing without leaking SFML asset details into gameplay modules.
- Add runtime asset wiring so the terrain tileset is available to the executable at run time.
- Keep non-terrain markers, world generation, and gameplay-owned render snapshot assembly unchanged in this change.

## Capabilities

### New Capabilities
- `overworld-tileset-rendering`: Render visible overworld terrain from a shared tileset texture using the existing tile-type-driven snapshot data.

### Modified Capabilities

## Impact

- Affected code: `src/main/Game.cpp`, `World`/`OverworldRuntime` tile geometry behavior, executable CMake/resource wiring, and any render-only helpers introduced for terrain tile drawing.
- Affected assets: a new terrain tileset image for the four current overworld tile types.
- Preserved boundaries: `World` and `OverworldRuntime` continue to expose repo-native tile data rather than SFML texture details.
