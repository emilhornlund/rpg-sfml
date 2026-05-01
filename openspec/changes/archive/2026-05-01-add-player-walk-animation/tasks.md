## 1. Gameplay presentation state

- [x] 1.1 Extend `Player` to track facing direction, movement state, and walk-frame timing/index without introducing SFML types.
- [x] 1.2 Add player sprite placement and animation metadata to the overworld render snapshot types and refresh logic.
- [x] 1.3 Replace the existing marker placement helper with a player sprite placement helper that anchors the padded frame to the tile-centered foot pivot.

## 2. Game-shell sprite rendering

- [x] 2.1 Stage `player-walking-spritesheet.png` through the executable asset copy step and load it alongside the terrain tileset.
- [x] 2.2 Update `Game.cpp` to select the correct player spritesheet row/frame from snapshot metadata and draw the player sprite instead of the rectangle marker.
- [x] 2.3 Keep terrain rendering unchanged while preserving the gameplay-owned render snapshot boundary for player presentation.

## 3. Verification

- [x] 3.1 Update runtime support and overworld tests to assert sprite placement semantics, facing/frame metadata, and removal of marker-specific assumptions.
- [x] 3.2 Update any asset-staging tests or checks that currently assume only the terrain tileset is copied to the build asset directory.
- [x] 3.3 Run the documented build and test commands to confirm the sprite-backed player rendering change is wired correctly.
