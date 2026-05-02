## MODIFIED Requirements

### Requirement: Render snapshots describe visible frame content
The render snapshot SHALL include the active camera frame, the visible terrain tiles for that frame, a collection of renderable markers for gameplay entities that should appear in the frame, and a collection of renderable generated-content entries for deterministic world content visible in that frame.

#### Scenario: Snapshot carries terrain, marker, and generated-content data
- **WHEN** the overworld runtime prepares the current frame for rendering
- **THEN** the snapshot includes visible tile entries covering the active camera-bounded terrain view
- **AND** it includes marker entries for the player and any other renderable entities that belong in the frame
- **AND** it includes generated-content entries for deterministic world content visible in that frame
- **AND** it includes the camera frame used to render the snapshot

### Requirement: Snapshot entries are render-ready without gameplay reinterpretation
Each visible tile entry, marker entry, and generated-content entry in the render snapshot SHALL provide the world-space geometry and presentation identifiers needed for drawing so the outer game shell can render the frame without re-deriving gameplay presentation rules from simulation state. Player-facing snapshot data SHALL include the placement and animation selectors needed to choose the correct player sprite frame from the bundled walking spritesheet without requiring the game shell to infer facing or movement presentation from raw gameplay state. Generated-content-facing snapshot data SHALL include the identity, footprint, and opaque appearance selectors needed to draw deterministic world content without requiring the shell to inspect `World` or raw chunk-content records directly.

#### Scenario: Game renders directly from snapshot entries
- **WHEN** the outer game shell renders an overworld frame from the published snapshot
- **THEN** it uses the snapshot's tile entries, marker entries, and generated-content entries as the source of draw data
- **AND** it does not infer additional overworld presentation state by inspecting gameplay-owned modules
- **AND** it can choose the correct player spritesheet frame from snapshot-provided player presentation metadata
- **AND** it can draw deterministic generated content from snapshot-provided content presentation metadata
