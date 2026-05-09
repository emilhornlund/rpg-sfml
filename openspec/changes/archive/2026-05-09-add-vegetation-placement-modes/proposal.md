## Why

The current vegetation system only distinguishes between `tree` and non-tree placement behavior, which forces bushes, water plants, stumps, and logs into the same dense spawn path. That makes forest debris frequency hard to tune and makes small biome weights feel misleading because they act only as prototype weights inside an overly broad pool.

## What Changes

- Add explicit vegetation `placementMode` metadata so staged vegetation prototypes can declare how they should be sampled instead of relying on `family`, tags, or prototype names.
- Update vegetation metadata loading and runtime placement to support three modes: `tree_sparse`, `ground_dense`, and `prop_sparse`.
- Keep existing biome weights as prototype-selection weights within each placement mode rather than treating them as global spawn probabilities.
- Introduce a dedicated sparse placement path for occasional forest props such as stumps and logs while preserving dense ground cover for bushes, reeds, and lilies.
- Add focused validation for loaded placement modes and for the resulting forest-density hierarchy: many trees, some ground cover, occasional props.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `vegetation-content`: vegetation placement behavior changes from a tree-vs-non-tree split to explicit placement-mode-driven sampling with distinct density behavior for trees, dense ground cover, and sparse props.
- `tileset-asset-loading`: vegetation classification loading preserves anchor `placementMode` metadata together with existing placement constraints and biome weights.

## Impact

- Affected specs: `openspec/specs/vegetation-content/spec.md`, `openspec/specs/tileset-asset-loading/spec.md`
- Affected code: `src/main/VegetationAtlasSupport.*`, `src/main/WorldContent.cpp`
- Affected assets: staged vegetation classification data under `assets/output/classifications/overworld-vegetation-tileset-classification.json`
- Likely affected tests: `tests/TilesetAssetLoaderTests.cpp`, `tests/WorldTerrainGeneratorTests.cpp`
- No public API or dependency changes are expected
