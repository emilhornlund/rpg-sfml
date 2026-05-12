## 1. Render batch helpers

- [x] 1.1 Add render-side helpers that build batched terrain geometry from `OverworldRenderSnapshot::visibleTiles` using the existing terrain tileset metadata and autotile selection path.
- [x] 1.2 Add render-side helpers that build batched tile-grid geometry from the existing tile-grid overlay rectangle logic so grid thickness and alignment stay unchanged.

## 2. Game rendering integration

- [x] 2.1 Replace per-tile terrain sprite submission in `Game::render()` with the new terrain batch draw path while preserving the current camera/view setup.
- [x] 2.2 Replace per-rectangle tile-grid submission in `Game::render()` with the new batched grid draw path while preserving the existing debug toggle behavior and render ordering.
- [x] 2.3 Keep gameplay/render boundaries intact by containing SFML vertex-array ownership and batch construction within the render shell and render-side helpers.

## 3. Validation

- [x] 3.1 Update rendering-boundary tests to cover batched terrain and batched grid submission without breaking current tileset and debug-grid behavior.
- [x] 3.2 Run the documented build and test commands and verify the terrain/grid batching path remains correct across resize and zoom-driven rendering scenarios.
