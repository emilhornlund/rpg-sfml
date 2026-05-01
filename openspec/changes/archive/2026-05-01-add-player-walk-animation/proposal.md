## Why

The overworld currently renders the player as a simple marker, which does not match the existing terrain presentation or the available player art asset. Adding textured player rendering and directional walk animation will make movement readable in-world and establish a reusable boundary for sprite-driven entity presentation.

## What Changes

- Replace the overworld player marker with sprite-backed rendering driven by the existing player walking spritesheet.
- Add gameplay-owned player presentation state for facing direction, movement state, and current walk frame selection without leaking SFML types into gameplay modules.
- Extend render snapshot data so the game shell can draw the player from sprite/animation metadata rather than hard-coded marker visuals.
- Define stable alignment rules for padded player frames so the sprite remains centered on a 16x16 world tile while extending upward into the tile above.
- Stage the player spritesheet alongside the existing terrain tileset in the executable asset output.

## Capabilities

### New Capabilities
- `player-sprite-animation`: Define directional player sprite presentation, walk-cycle selection, and tile-aligned placement for the overworld player.

### Modified Capabilities
- `render-snapshots`: Player-facing snapshot entries need to carry sprite animation metadata and placement geometry that the game shell can draw directly.
- `overworld-vertical-slice`: The overworld slice should render the player as an animated sprite aligned to the world tile baseline instead of a basic marker.

## Impact

- Affected specs: `player-sprite-animation`, `render-snapshots`, `overworld-vertical-slice`
- Affected code: `include/main/Player.hpp`, `src/main/Player.cpp`, `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, `src/main/Game.cpp`, `src/main/GameRuntimeSupport.hpp`, `src/main/CMakeLists.txt`, and relevant tests under `tests/`
- Assets: `src/main/assets/player-walking-spritesheet.png`
