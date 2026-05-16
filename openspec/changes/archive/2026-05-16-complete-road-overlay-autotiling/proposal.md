## Why

Road overlays currently stop at a thin one-tile cross around spawn, so the renderer cannot realize the wider dirt-road shapes the staged overlay tileset was authored to support. The updated catalog also distinguishes `base` and `decor` overlay classes, but runtime road selection still treats all non-transition dirt variations as the same pool, which prevents sparse decorative placement and wastes the authored texture variety.

## What Changes

- Expand deterministic road overlay membership from a one-tile centerline into roads that render at least two tiles wide and can widen to three tiles in deterministic segments.
- Update road overlay presentation so neighbor-driven transitions are used to shape wider roads against grass, forest, and sand instead of only rendering a thin strip with limited edge cases.
- Distinguish road overlay `base` and `decor` variants at runtime so interior road tiles use stable base texture variation while decor variants appear as sparse visual replacements.
- Support road-end and widening motifs by composing the existing transition tiles through occupancy-aware rendering instead of assuming every motif needs a dedicated single-tile atlas role.
- Refresh road overlay tests and metadata expectations to match the updated catalog semantics and widened-road behavior.

## Capabilities

### New Capabilities
- None.

### Modified Capabilities
- `overworld-road-overlays`: road overlay membership and rendering requirements expand to support wider deterministic roads, sparse decor variants, and composed transition motifs such as end caps.

## Impact

- Affected code: `src/main/World.cpp`, `src/main/WorldContent.cpp`, `src/main/GameRenderBatchSupport.cpp`, `src/main/RoadOverlayTilesetSupport.cpp`, and related tests under `tests/`.
- Affected assets: `assets/output/catalogs/overworld-ground-overlay-tileset-catalog.json` remains the staged source of truth for road overlay classes and transitions.
- Affected behavior: visible road overlay sets, atlas-cell selection for road rendering, and road-aware vegetation exclusion all change while remaining deterministic for the same seed and coordinates.
