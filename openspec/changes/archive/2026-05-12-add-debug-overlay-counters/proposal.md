## Why

The current debug overlay shows cache and object counts, but it does not expose the visibility and batch sizes that actually explain render slowdowns when the camera shows many tiles. Adding a few targeted counters will make it easier to see whether a slowdown scales with visible terrain, visible content, front-layer occluders, or generated vertex counts.

## What Changes

- Expand the debug overlay diagnostics to show visible tile count, visible generated-content count, front occluder count, terrain vertex count, and tile-grid vertex count in addition to the existing FPS, cache, coordinate, and zoom values.
- Extend gameplay-owned debug snapshot data so the overlay can read visible tile and visible generated-content counts without querying `World` or `OverworldRuntime` internals directly from the game shell.
- Keep front-occluder and vertex-array counters in the render shell so render-only diagnostics continue to be sourced where they are computed.

## Capabilities

### New Capabilities

### Modified Capabilities
- `debug-overlay-display`: The overlay diagnostics will include counters that explain visibility scale and render-batch size for the active frame.
- `render-snapshots`: The published debug snapshot will include gameplay-owned visibility counts needed by the overlay.

## Impact

- Affected code: `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, `src/main/GameRuntimeSupport.hpp`, `src/main/Game.cpp`, and overlay/runtime tests under `tests/`.
- Affected systems: debug overlay presentation, render snapshot diagnostics, and render-shell debug instrumentation.
- No intended public API, asset, or dependency changes.
