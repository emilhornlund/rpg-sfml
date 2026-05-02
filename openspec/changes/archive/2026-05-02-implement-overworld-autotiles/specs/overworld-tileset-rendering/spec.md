## MODIFIED Requirements

### Requirement: Game renders visible terrain from a shared overworld tileset
The game runtime SHALL draw visible overworld terrain tiles from a shared terrain tileset texture and its associated classification metadata instead of filling per-tile rectangle shapes with hard-coded terrain colors or using a fixed atlas lookup embedded in `Game.cpp`.

#### Scenario: Visible terrain uses metadata-backed tileset drawing
- **WHEN** the active game loop renders an overworld frame containing visible terrain tiles
- **THEN** it draws those terrain tiles using the shared terrain tileset texture selected for the overworld
- **AND** it uses classification metadata to resolve the atlas entry for each tile while still positioning tiles from the render snapshot's world-space geometry

### Requirement: Tile types map deterministically to terrain tileset cells
The game runtime SHALL translate each visible overworld tile into a deterministic terrain tileset selection from repo-native terrain categories, neighboring terrain categories, and updated atlas metadata so grass, sand, water, and forest terrain render through stable base variants, transition roles, and water animation frames without requiring gameplay modules to provide atlas coordinates or SFML texture metadata.

#### Scenario: Terrain rendering stays deterministic for the same visible layout
- **WHEN** the game shell renders the same visible terrain layout for the same world state
- **THEN** it selects the same base variants and autotile roles for the matching tiles on every frame
- **AND** the only permitted visual frame-to-frame change for unchanged terrain is the active animation frame of water-targeting transitions

### Requirement: Terrain tileset integration preserves the gameplay render boundary
The terrain tileset integration SHALL preserve the existing gameplay-owned render snapshot boundary so gameplay-facing modules continue to publish repo-native terrain identifiers rather than rendering-backend-specific asset details, and the SFML shell SHALL own the updated terrain texture and classification metadata lookup needed to render those identifiers.

#### Scenario: Gameplay modules stay texture-agnostic with metadata-backed rendering
- **WHEN** the autotile tileset rendering capability is introduced
- **THEN** `World` and `OverworldRuntime` continue to expose visible terrain through repo-native tile entries and `TileType` identifiers
- **AND** SFML texture ownership, JSON classification loading, and atlas lookup remain in the outer game shell or render-side helpers
