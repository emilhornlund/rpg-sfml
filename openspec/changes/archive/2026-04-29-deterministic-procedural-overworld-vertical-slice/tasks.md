## 1. World and gameplay state

- [x] 1.1 Extend `World` to generate and store a deterministic finite tile grid, tile classifications, and a traversable spawn position.
- [x] 1.2 Extend `Player` to own player position and movement updates constrained by world bounds and traversable tiles.
- [x] 1.3 Extend `Camera` to derive player-follow framing and clamp the visible view to the world extents.

## 2. Runtime integration and rendering

- [x] 2.1 Wire `Game` update flow so startup initializes the overworld slice and frame updates apply player movement and camera tracking through the dedicated modules.
- [x] 2.2 Wire `Game` render flow to draw visible world tiles with basic built-in visuals and render the player marker through the active camera framing.
- [x] 2.3 Keep deterministic simulation and coordinate helpers in module code or small runtime support helpers instead of pushing gameplay rules into `Game`.

## 3. Verification

- [x] 3.1 Add or update tests that cover deterministic world generation, spawn validity, player movement constraints, and camera clamping behavior.
- [x] 3.2 Add or update tests and runtime checks that confirm the overworld slice renders through the existing game loop without changing the public `Game` boundary.
- [x] 3.3 Run the documented build and CTest commands and resolve any issues introduced by the vertical slice implementation.
