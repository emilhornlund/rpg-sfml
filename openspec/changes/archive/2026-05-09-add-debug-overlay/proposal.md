## Why

The overworld runtime already exposes debug-oriented controls such as zoom and tile-grid rendering, but it does not provide an in-game way to inspect runtime state while the scene is running. A lightweight debug overlay would make it easier to reason about rendering, camera scale, and streamed world content during development without interrupting normal input handling.

## What Changes

- Add a toggleable debug overlay that can be shown and hidden with `F1` while the game continues to process movement and other debug controls.
- Render the overlay as a semi-transparent black panel with readable text showing frame rate, loaded generated-content count, rendered generated-content count, player tile coordinates, and zoom level.
- Publish the gameplay-derived debug values through repo-native runtime data so the SFML shell can render the overlay without querying gameplay modules directly.
- Stage one of the bundled font assets for runtime loading so the overlay text can be rendered from the executable-local asset directory.

## Capabilities

### New Capabilities
- `debug-overlay-display`: Covers the toggleable in-game debug HUD, its visible fields, and its non-blocking presentation behavior.

### Modified Capabilities
- `input-boundary`: Add the `F1` overlay toggle to the shell-owned debug input translation rules while preserving repo-native input beyond the shell boundary.
- `render-snapshots`: Extend gameplay-published runtime snapshots with the debug-facing values needed to render the overlay without shell-side gameplay inspection.
- `runtime-asset-layout`: Stage the runtime font asset required to render the overlay text from the built executable's `assets/` directory.

## Impact

- Affected specs: `input-boundary`, `render-snapshots`, `runtime-asset-layout`, and new `debug-overlay-display`
- Affected code: `src/main/Game.cpp`, `src/main/GameAssetSupport.hpp`, `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, and related tests/build asset staging
- Runtime behavior: adds a developer-facing overlay and font loading path without changing gameplay movement or pause behavior
