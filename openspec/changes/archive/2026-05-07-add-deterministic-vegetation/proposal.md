## Why

The current world-content path derives only a couple of deterministic instances per chunk from coarse chunk metadata, which is too sparse and too chunk-shaped for forests, scattered grassland trees, and other vegetation. The project now has a vegetation tileset and classification data, so this is the right time to introduce a dedicated vegetation system that samples placement from stable world-space inputs and renders it with depth-aware ordering.

## What Changes

- Add deterministic vegetation generation for overworld trees and small vegetation using world-space sampling driven by seed-backed hash or noise fields instead of runtime randomness.
- Introduce biome-sensitive vegetation placement rules so forest areas produce dense, clustered vegetation while grass areas produce sparse, isolated vegetation.
- Add runtime vegetation metadata loading for the staged overworld vegetation tileset classification and use that metadata to resolve vegetation atlas content.
- Publish vegetation through the existing world/runtime render pipeline as deterministic world content with stable identity, placement, geometry, and appearance selection.
- Change overworld object rendering so vegetation and the player can be rendered with y-sorted ordering instead of a fixed "all generated content below player" layer.

## Capabilities

### New Capabilities
- `vegetation-content`: Deterministic vegetation sampling, biome-aware placement, vegetation metadata resolution, and world-facing vegetation records for render and gameplay consumers.

### Modified Capabilities
- `generated-content-rendering`: Generated world content rendering changes from a fixed content layer below the player to y-sorted object rendering for vegetation and player presentation.
- `render-snapshots`: Render snapshots publish vegetation-ready content entries and object ordering data needed for y-sorted presentation.
- `runtime-asset-layout`: The build stages the overworld vegetation tileset image and classification document as executable runtime assets.

## Impact

- Affected code: `src/main/WorldContent.*`, `src/main/World.*`, `src/main/OverworldRuntime.*`, `src/main/Game.*`, asset support helpers, and vegetation metadata support alongside the existing terrain metadata path.
- Affected assets: `assets/overworld-vegetation-tileset.png` and `assets/output/classifications/overworld-vegetation-tileset-classification.json`.
- Affected tests: world-content, runtime snapshot, rendering support, asset staging/loading, and deterministic generation coverage.
