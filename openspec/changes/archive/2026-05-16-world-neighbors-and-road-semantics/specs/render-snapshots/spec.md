## MODIFIED Requirements

### Requirement: Render snapshots publish visible road overlay entries
The overworld runtime SHALL publish visible road overlay entries for the active camera frame as part of the gameplay-owned render snapshot so the outer game shell can render road overlays without querying `World` directly. Each published road overlay entry SHALL also carry world-backed structural neighbor context for the surrounding stamped road field so final overlay selection does not depend on which neighboring tiles happened to be included in the visible subset for the current frame.

#### Scenario: Snapshot carries visible road overlay geometry and surface context
- **WHEN** the overworld runtime prepares a render snapshot for a frame containing visible roads
- **THEN** the snapshot includes visible road overlay entries covering the active camera-bounded area
- **AND** each entry provides the world-space geometry and underlying terrain context needed to render the overlay tile
- **AND** the outer game shell does not need to inspect `World` directly to discover road-covered tiles for that frame

#### Scenario: Road overlay rendering stays correct at viewport edges
- **WHEN** a visible road overlay tile has stamped-road neighbors outside the current visible snapshot bounds
- **THEN** the published road overlay entry still includes the world-backed structural neighbor context needed to resolve its final overlay selection
- **AND** the renderer does not need to infer missing road neighbors from the visible overlay subset alone

### Requirement: Snapshot entries are render-ready without gameplay reinterpretation
Each visible tile entry, road overlay entry, marker entry, and generated-content entry in the render snapshot SHALL provide the world-space geometry and presentation identifiers needed for drawing so the outer game shell can render the frame without re-deriving gameplay presentation rules from simulation state. Terrain-facing snapshot data SHALL include the neighboring terrain categories needed to resolve deterministic terrain autotiling from the world source of truth, including neighbors outside the current visible subset. Player-facing snapshot data SHALL include the placement and animation selectors needed to choose the correct player sprite frame from the bundled walking spritesheet without requiring the game shell to infer facing or movement presentation from raw gameplay state. Generated-content-facing snapshot data SHALL include the identity, footprint, opaque appearance selectors, and ordering data needed to draw deterministic world content without requiring the shell to inspect `World` or raw chunk-content records directly.

#### Scenario: Game renders directly from snapshot entries
- **WHEN** the outer game shell renders an overworld frame from the published snapshot
- **THEN** it uses the snapshot's tile entries, road overlay entries, marker entries, and generated-content entries as the source of draw data
- **AND** it does not infer additional overworld presentation state by inspecting gameplay-owned modules
- **AND** it can choose the correct player spritesheet frame from snapshot-provided player presentation metadata
- **AND** it can draw deterministic generated content from snapshot-provided content presentation metadata
- **AND** it can order vegetation and player presentation from snapshot-provided world-space ordering data

#### Scenario: Terrain rendering stays correct at viewport edges
- **WHEN** a visible terrain tile has one or more off-screen neighboring terrain tiles that affect its autotile transition
- **THEN** the published visible tile entry still carries the neighboring terrain categories needed for deterministic atlas selection
- **AND** the renderer does not substitute the current tile type for those off-screen neighbors just because they are outside the visible subset
