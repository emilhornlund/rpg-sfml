## Purpose

Define overworld terrain rendering through a shared tileset-backed runtime boundary while keeping gameplay modules texture-agnostic.

## Requirements

### Requirement: Game renders visible terrain from a shared overworld tileset
The game runtime SHALL draw visible overworld terrain tiles from a shared terrain tileset texture and its associated classification metadata instead of filling per-tile rectangle shapes with hard-coded terrain colors or using a fixed atlas lookup embedded in `Game.cpp`. The render shell SHALL be allowed to submit the visible terrain as one batched textured layer for the frame instead of issuing one terrain sprite draw per visible tile.

#### Scenario: Visible terrain uses metadata-backed tileset drawing
- **WHEN** the active game loop renders an overworld frame containing visible terrain tiles
- **THEN** it draws those terrain tiles using the shared terrain tileset texture selected for the overworld
- **AND** it uses classification metadata resolved through the runtime tileset asset loading boundary to determine the atlas entry for each tile while still positioning tiles from the render snapshot's world-space geometry
- **AND** the render shell may submit the resulting terrain as batched textured geometry rather than separate terrain sprite draws

### Requirement: Tile types map deterministically to terrain tileset cells
The game runtime SHALL translate each visible overworld tile into a deterministic terrain tileset selection from repo-native terrain categories, neighboring terrain categories, and updated atlas metadata so grass, sand, water, and forest terrain render through stable base variants, transition roles, and water animation frames without requiring gameplay modules to provide atlas coordinates or SFML texture metadata.

#### Scenario: Terrain rendering stays deterministic for the same visible layout
- **WHEN** the game shell renders the same visible terrain layout for the same world state
- **THEN** it selects the same base variants and autotile roles for the matching tiles on every frame
- **AND** the only permitted visual frame-to-frame change for unchanged terrain is the active animation frame of water-targeting transitions

### Requirement: Overworld terrain geometry uses a 16-unit tile baseline
The overworld runtime SHALL use 16x16 world units as the tile geometry baseline for visible terrain and tile-derived marker sizing so the runtime scale matches the intended 16x16 tileset baseline.

#### Scenario: Render snapshot reflects 16-unit terrain geometry
- **WHEN** the overworld runtime publishes a render snapshot after the tile-size change
- **THEN** each visible terrain tile entry reports 16x16 world-space geometry
- **AND** tile-derived origin and marker sizing reflect that 16-unit tile baseline consistently

### Requirement: Terrain tileset integration preserves the gameplay render boundary
The terrain tileset integration SHALL preserve the existing gameplay-owned render snapshot boundary so gameplay-facing modules continue to publish repo-native terrain identifiers rather than rendering-backend-specific asset details, and the SFML shell SHALL own the reusable tileset asset loading, texture ownership, atlas lookup, and terrain-batch construction needed to render those identifiers.

#### Scenario: Gameplay modules stay texture-agnostic with metadata-backed rendering
- **WHEN** the autotile tileset rendering capability is introduced
- **THEN** `World` and `OverworldRuntime` continue to expose visible terrain through repo-native tile entries and `TileType` identifiers
- **AND** reusable tileset loading, JSON classification parsing, atlas lookup, and terrain-batch construction remain in the outer game shell or render-side helpers
