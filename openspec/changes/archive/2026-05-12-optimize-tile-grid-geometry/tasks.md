## 1. Grid strip builder

- [x] 1.1 Replace the per-tile edge expansion in `buildTileGridVertexArray(...)` with bounds-derived horizontal and vertical strip generation from the visible tile set.
- [x] 1.2 Preserve the current grid appearance by keeping outer borders at the configured line thickness and interior boundaries visually equivalent to the existing doubled-edge result.

## 2. Rendering and metrics

- [x] 2.1 Keep the existing `Game::render()` grid-toggle flow unchanged while wiring it to the optimized grid vertex array output.
- [x] 2.2 Ensure debug overlay metrics report the optimized tile-grid vertex count for active frames and zero when the grid overlay is disabled.

## 3. Validation

- [x] 3.1 Update rendering-focused tests to cover multi-tile grid batching, including the reduced rectangle count and preserved world-space alignment.
- [x] 3.2 Run the documented build and test commands to confirm the optimized tile-grid batching path integrates cleanly with the current renderer.
