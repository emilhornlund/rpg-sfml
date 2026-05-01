## Why

The overworld currently accepts diagonal movement intent and advances the player freely across pixel space. That conflicts with the four-direction walking presentation and makes the player drift off the tile grid instead of moving like a classic tile-based RPG.

## What Changes

- Restrict overworld movement to a single cardinal direction per step instead of allowing diagonal travel.
- Resolve simultaneous perpendicular inputs to one active movement direction using last-pressed-wins behavior.
- Change player locomotion from free pixel movement to tile-centered step movement between adjacent tile centers.
- Make the player finish the current tile step after input is released mid-tile, then stop at the destination tile center.
- Prevent new tile steps from starting when the destination tile is not traversable.
- Update automated tests to cover cardinal input resolution, step completion, and tile-grid alignment.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `input-boundary`: overworld input translation now resolves directional key combinations to a single cardinal movement direction suitable for grid-locked movement.
- `overworld-vertical-slice`: player movement changes from free movement within valid traversable space to tile-centered cardinal stepping with automatic step completion.

## Impact

- Affects `Game.cpp`, `GameRuntimeSupport.hpp`, `Player.hpp`, `Player.cpp`, and the overworld runtime path that applies player input.
- Requires updates to movement-oriented tests in `tests/GameRuntimeSupportTests.cpp`, `tests/OverworldRuntimeTests.cpp`, and `tests/OverworldVerticalSliceTests.cpp`.
- Changes moment-to-moment player control feel, collision handling, and the movement state consumed by sprite-facing presentation.
