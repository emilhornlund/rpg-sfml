## Why

The current default overworld framing is too zoomed out for a 16x16 tile grid, making the game feel distant and hard to read in normal play as well as during development. Moving the default view to a tighter 300% zoom and keeping debug-only zoom/grid tools on top of that makes the overworld more playable while preserving useful development diagnostics.

## What Changes

- Make 300% the default overworld zoom level for all modes, and keep debug-only zoom controls with 50% increments for development and testing.
- Add a debug-only tile-grid overlay that renders above terrain and below the player.
- Scale overworld player movement speed relative to the active debug zoom so traversal keeps a roughly stable on-screen pace.
- Slow the baseline player movement and walking animation cadence so traversal feels like walking rather than sprinting.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `input-boundary`: Extend overworld input translation to include debug-only hotkeys for zooming and toggling the tile-grid overlay while preserving the shell boundary.
- `overworld-vertical-slice`: Adjust overworld camera framing, keep 300% as the default view in all modes, and preserve zoom-relative traversal behavior for debug zoom controls.
- `player-sprite-animation`: Retune walking animation cadence to match the slower implemented movement pacing while preserving time-based animation behavior.

## Impact

- Affected code: `src/main/Game.cpp`, `src/main/GameRuntimeSupport.hpp`, `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, `include/main/Camera.hpp`, `src/main/Camera.cpp`, `include/main/Player.hpp`, `src/main/Player.cpp`, and relevant tests under `tests/`.
- No external API or dependency changes are expected.
- Keeps debug-only hotkeys and tile-grid overlay gated away from normal gameplay builds or modes while applying the 300% default camera framing everywhere.
