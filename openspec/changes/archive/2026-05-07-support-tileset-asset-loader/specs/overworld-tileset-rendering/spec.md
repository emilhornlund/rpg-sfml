## MODIFIED Requirements

### Requirement: Game renders visible terrain from a shared overworld tileset
The game runtime SHALL draw visible overworld terrain tiles from a shared terrain tileset texture and its associated classification metadata instead of filling per-tile rectangle shapes with hard-coded terrain colors or using a fixed atlas lookup embedded in `Game.cpp`.

#### Scenario: Visible terrain uses metadata-backed tileset drawing
- **WHEN** the active game loop renders an overworld frame containing visible terrain tiles
- **THEN** it draws those terrain tiles using the shared terrain tileset texture selected for the overworld
- **AND** it uses classification metadata resolved through the runtime tileset asset loading boundary to determine the atlas entry for each tile while still positioning tiles from the render snapshot's world-space geometry

### Requirement: Terrain tileset integration preserves the gameplay render boundary
The terrain tileset integration SHALL preserve the existing gameplay-owned render snapshot boundary so gameplay-facing modules continue to publish repo-native terrain identifiers rather than rendering-backend-specific asset details, and the SFML shell SHALL own the reusable tileset asset loading, texture ownership, and atlas lookup needed to render those identifiers.

#### Scenario: Gameplay modules stay texture-agnostic with metadata-backed rendering
- **WHEN** the autotile tileset rendering capability is introduced
- **THEN** `World` and `OverworldRuntime` continue to expose visible terrain through repo-native tile entries and `TileType` identifiers
- **AND** reusable tileset loading, JSON classification parsing, and atlas lookup remain in the outer game shell or render-side helpers
