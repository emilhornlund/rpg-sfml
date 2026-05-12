## 1. Snapshot diagnostics

- [x] 1.1 Extend `OverworldDebugSnapshot` with visible tile and visible generated-content counters.
- [x] 1.2 Populate the new snapshot counters in `OverworldRuntime::refreshRenderSnapshot()` from the published render snapshot collections.

## 2. Overlay integration

- [x] 2.1 Add render-side overlay inputs for front occluder, terrain vertex, and tile-grid vertex counts while preserving the current gameplay/render boundary.
- [x] 2.2 Update the debug overlay string builder and `Game::render()` to display the expanded counter set, using zero grid vertices when the tile grid is disabled.

## 3. Validation

- [x] 3.1 Update runtime and overlay tests to cover the new snapshot fields and debug overlay text.
- [x] 3.2 Run the documented build and test commands to confirm the expanded diagnostics integrate cleanly.
