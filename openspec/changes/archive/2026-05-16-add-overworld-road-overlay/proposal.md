## Why

The new overworld ground overlay tileset provides a strong next step for improving map readability and variety without taking on the much larger structural and occlusion work implied by cliff-style occluders. A road overlay can fit the current world-and-render architecture if it is introduced as a deterministic secondary tile layer that preserves the existing gameplay/render boundary.

## What Changes

- Add deterministic overworld dirt-road overlays as a secondary visual tile layer rendered above terrain and below vegetation and player markers.
- Extend the staged tileset asset loading boundary so runtime code can load overlay-classified catalog entries in addition to terrain and vegetation entries.
- Publish visible road overlay entries through the gameplay-owned render snapshot so the game shell can render overlays without querying `World` directly.
- Prevent deterministic ground vegetation placement from appearing on road-covered tiles so the first road-overlay slice reads cleanly in play.

## Capabilities

### New Capabilities
- `overworld-road-overlays`: Deterministic road overlay data, metadata resolution, and rendering behavior for the staged dirt-road tileset.

### Modified Capabilities
- `tileset-asset-loading`: Extend the shared tileset catalog loading boundary to preserve overlay-classified tile metadata from staged assets.
- `render-snapshots`: Expand the gameplay-owned render snapshot contract to publish visible road overlay entries for the current frame.
- `vegetation-content`: Ensure deterministic vegetation placement respects road-covered tiles and does not spawn ground cover onto visible roads.

## Impact

- Affected code: `src/main/TilesetAssetLoader.*`, `src/main/GameAssetSupport.hpp`, `src/main/GameResourceBootstrapSupport.*`, `src/main/GameRenderBatchSupport.*`, `src/main/GameSceneRenderSupport.*`, `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, `include/main/World.hpp`, `src/main/World.cpp`, `src/main/WorldContent.cpp`, and new overlay metadata/render helpers.
- Affected assets: `assets/output/catalogs/overworld-ground-overlay-tileset-catalog.json` and its runtime image.
- Affected behavior: overworld frames gain a road overlay layer and vegetation generation becomes road-aware where overlays are present.
