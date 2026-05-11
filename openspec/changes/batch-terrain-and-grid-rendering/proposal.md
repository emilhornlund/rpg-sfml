## Why

Resizing to larger display modes and zooming the camera out increases the number of visible tiles substantially, and the current renderer submits terrain and tile-grid primitives one item at a time. The recent investigation showed the largest reproducible frame-time regression comes from the debug tile grid, which turns one visible terrain layer into thousands of small SFML draw calls.

## What Changes

- Batch visible terrain into shared vertex-array-backed geometry instead of issuing one sprite draw per visible tile.
- Batch the debug tile grid overlay into shared vertex-array-backed geometry instead of issuing four rectangle draws per visible tile.
- Preserve the current terrain autotile selection, world-space tile geometry, zoom behavior, and pixel-stable camera presentation while changing how the render shell submits geometry to SFML.
- Keep the gameplay/render boundary intact so batching remains an SFML-shell concern and does not leak SFML vertex details into `World` or `OverworldRuntime`.
- Preserve existing debug controls and overlay semantics, including tile-grid toggling through the debug view.

## Capabilities

### New Capabilities
- `batched-overworld-geometry`: Defines the render-shell batching behavior for terrain and debug tile-grid geometry so large visible tile counts do not require per-tile draw submission.

### Modified Capabilities
- `overworld-tileset-rendering`: Terrain rendering will preserve its current tileset-driven output while allowing the shell to submit the visible terrain as batched geometry.
- `debug-overlay-display`: Tile-grid visualization will preserve its current toggle behavior and visual alignment while allowing the shell to submit the grid as batched geometry.

## Impact

- Affected code: `src/main/Game.cpp`, render-side helpers under `src/main/`, and rendering-focused tests in `tests/`.
- Affected systems: overworld terrain drawing, debug tile-grid overlay drawing, and any render helper logic that currently builds per-tile SFML drawables.
- No intended public API changes for gameplay modules or asset formats.
