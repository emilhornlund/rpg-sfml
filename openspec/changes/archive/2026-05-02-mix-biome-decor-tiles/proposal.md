## Why

The overworld terrain renderer already supports deterministic autotiles and stable base variation, but non-transition terrain still looks too uniform once the same biome extends across large areas. The terrain tileset classification metadata already includes biome-specific decor tiles, so the renderer can add more visual detail now without changing gameplay-facing terrain data.

## What Changes

- Update overworld terrain presentation so non-transition grass, sand, and forest tiles deterministically choose from both biome base variants and biome decor variants.
- Keep decor selection procedural and stable for the same world seed, tile coordinates, biome, and local terrain neighborhood.
- Preserve existing transition behavior so autotile transitions always override base-or-decor fallback selection.
- Keep decor as a visual replacement for base terrain rendering only, with no changes to `TileType`, traversability, chunk metadata, or gameplay-facing render snapshot structures.

## Capabilities

### New Capabilities
- None.

### Modified Capabilities
- `overworld-autotile-rendering`: Expand deterministic non-transition terrain variation so biome decor tiles participate in the fallback base appearance selection with a low, stable frequency.

## Impact

- Affected specs: `openspec/specs/overworld-autotile-rendering/spec.md`
- Affected code: terrain tileset metadata loading, atlas-cell selection helpers, terrain rendering tests, and classification-driven variant selection logic in the game shell
- No gameplay API changes, asset format replacement, or dependency changes
