## ADDED Requirements

### Requirement: Game renders visible terrain from a shared overworld tileset
The game runtime SHALL draw visible overworld terrain tiles from a shared tileset texture instead of filling per-tile rectangle shapes with hard-coded terrain colors.

#### Scenario: Visible terrain uses tileset-backed drawing
- **WHEN** the active game loop renders an overworld frame containing visible terrain tiles
- **THEN** it draws those terrain tiles using a shared terrain tileset texture
- **AND** it uses each tile entry's world-space geometry from the render snapshot to position the drawn tile

### Requirement: Tile types map deterministically to terrain tileset cells
The game runtime SHALL translate each visible overworld `TileType` into a deterministic terrain tileset cell so the existing grass, sand, water, and forest categories render through stable atlas selections.

#### Scenario: Four terrain categories map to fixed atlas cells
- **WHEN** the game shell renders visible tiles whose snapshot entries identify grass, sand, water, or forest terrain
- **THEN** it selects the same predefined tileset cell for each matching terrain category on every frame
- **AND** it does not require gameplay modules to provide atlas coordinates or SFML texture metadata

### Requirement: Overworld terrain geometry uses a 16-unit tile baseline
The overworld runtime SHALL use 16x16 world units as the tile geometry baseline for visible terrain and tile-derived marker sizing so the runtime scale matches the intended 16x16 tileset baseline.

#### Scenario: Render snapshot reflects 16-unit terrain geometry
- **WHEN** the overworld runtime publishes a render snapshot after the tile-size change
- **THEN** each visible terrain tile entry reports 16x16 world-space geometry
- **AND** tile-derived origin and marker sizing reflect that 16-unit tile baseline consistently

### Requirement: Terrain tileset integration preserves the gameplay render boundary
The terrain tileset integration SHALL preserve the existing gameplay-owned render snapshot boundary so gameplay-facing modules continue to publish repo-native terrain identifiers rather than rendering-backend-specific asset details.

#### Scenario: Gameplay modules stay texture-agnostic
- **WHEN** the terrain tileset rendering capability is introduced
- **THEN** `World` and `OverworldRuntime` continue to expose visible terrain through repo-native tile entries and `TileType` identifiers
- **AND** SFML texture ownership and atlas lookup remain in the outer game shell
