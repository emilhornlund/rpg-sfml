## ADDED Requirements

### Requirement: Overworld runtime publishes render snapshots
The overworld gameplay runtime SHALL publish a render snapshot for the active frame instead of requiring the outer game shell to derive overworld presentation state ad hoc. The render snapshot SHALL be expressed through repo-native runtime types and SHALL not expose SFML-specific types.

#### Scenario: Snapshot is available after frame progression
- **WHEN** the overworld runtime initializes or updates the active gameplay frame
- **THEN** it publishes a render snapshot for that frame
- **AND** the snapshot can be consumed by the outer game shell without querying `World`, `Player`, or `Camera` directly

### Requirement: Render snapshots describe visible frame content
The render snapshot SHALL include the active camera frame, the visible terrain tiles for that frame, and a collection of renderable markers for gameplay entities that should appear in the frame.

#### Scenario: Snapshot carries terrain and marker data
- **WHEN** the overworld runtime prepares the current frame for rendering
- **THEN** the snapshot includes visible tile entries covering the active camera-bounded terrain view
- **AND** it includes marker entries for the player and any other renderable entities that belong in the frame
- **AND** it includes the camera frame used to render the snapshot

### Requirement: Snapshot entries are render-ready without gameplay reinterpretation
Each visible tile entry and marker entry in the render snapshot SHALL provide the world-space geometry and presentation identifiers needed for drawing so the outer game shell can render the frame without re-deriving gameplay presentation rules from simulation state.

#### Scenario: Game renders directly from snapshot entries
- **WHEN** the outer game shell renders an overworld frame from the published snapshot
- **THEN** it uses the snapshot's tile entries and marker entries as the source of draw data
- **AND** it does not infer additional overworld presentation state by inspecting gameplay-owned modules
