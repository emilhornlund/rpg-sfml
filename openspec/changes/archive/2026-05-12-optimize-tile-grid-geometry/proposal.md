## Why

The debug tile-grid overlay is already batched, but it still emits four filled quads per visible tile by rebuilding each tile edge independently. On large visible areas this duplicates every interior edge and inflates grid geometry far beyond what the rectangular camera query actually requires.

## What Changes

- Rebuild the debug tile-grid overlay from the rectangular bounds implied by the visible tile set instead of emitting four quads per visible tile.
- Emit one horizontal strip per visible row boundary and one vertical strip per visible column boundary, reducing geometry from `4 * W * H` rectangles to `(W + 1) + (H + 1)` rectangles for a visible area of `W` columns by `H` rows.
- Preserve the current debug tile-grid toggle behavior, camera alignment, and visual thickness semantics while changing how the render shell constructs the shared grid geometry.
- Keep the optimization inside render-side helpers so `World` and `OverworldRuntime` remain unchanged and SFML-agnostic.

## Capabilities

### New Capabilities
- None.

### Modified Capabilities
- `debug-overlay-display`: the tile-grid overlay requirements will explicitly cover bounds-derived strip batching that preserves the current aligned debug-grid presentation while avoiding duplicated interior edge geometry.

## Impact

- Affected code: `src/main/GameRenderBatchSupport.cpp`, related render-side helper declarations, and rendering-focused tests under `tests/`.
- Affected systems: debug tile-grid geometry construction, debug overlay metrics, and rendering-boundary coverage for batched grid submission.
- No intended public API, gameplay-module, asset-format, or dependency changes.
