## Why

The overworld renderer still maps each terrain category to a single fixed atlas cell, which cannot use the updated tileset's terrain variations, neighbor-aware autotiles, or animated water transitions. The repo now includes an updated terrain atlas and tile classification JSON, so the runtime needs a deterministic autotile selection layer that can render those assets without pushing texture concerns into gameplay modules.

## What Changes

- Replace the fixed `TileType -> atlas cell` terrain rendering path with a render-time autotile selection layer driven by neighbor terrain relationships.
- Load terrain atlas metadata from the updated tileset classification JSON instead of hardcoding sprite rectangles in `Game.cpp`.
- Support deterministic base-tile variation for grass, sand, and forest while preserving stable rendering for a given world layout.
- Support directed terrain transition rendering for `grass->sand`, `forest->sand`, `grass->forest`, `grass->water`, `forest->water`, and `sand->water`.
- Support animated water transition frames while keeping world generation and gameplay-facing terrain ownership unchanged.
- Preserve the existing world/runtime boundary so `World`, `OverworldRuntime`, and render snapshots continue to expose repo-native terrain categories instead of atlas coordinates.

## Capabilities

### New Capabilities
- `overworld-autotile-rendering`: Define JSON-driven autotile selection, deterministic terrain variation, and animated water transition behavior for the updated overworld tileset.

### Modified Capabilities
- `overworld-tileset-rendering`: Replace the fixed per-`TileType` atlas-cell mapping requirement with updated tileset asset loading and an atlas metadata integration boundary that supports autotile lookup.

## Impact

- Affected code: `src/main/Game.cpp`, `src/main/assets/`, and new or updated runtime helpers for tileset metadata and autotile selection.
- Affected specs: `openspec/specs/overworld-tileset-rendering/spec.md` plus a new `openspec/specs/overworld-autotile-rendering/spec.md`.
- Affected assets: `overworld-terrain-tileset.png` and `overworld-terrain-tileset-classification.json`.
- No gameplay API or world-generation behavior is expected to change; the main impact is render-time terrain presentation.
