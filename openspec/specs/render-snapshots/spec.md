## Purpose

Define the gameplay-owned render snapshot boundary that supplies overworld frame content to the game shell without exposing SFML-specific runtime types.

## Requirements

### Requirement: Overworld runtime publishes render snapshots
The overworld gameplay runtime SHALL publish a render snapshot for the active frame instead of requiring the outer game shell to derive overworld presentation state ad hoc. The render snapshot SHALL be expressed through repo-native runtime types and SHALL not expose SFML-specific types. The overworld runtime SHALL also publish repo-native debug snapshot data for the active frame so the outer game shell can render diagnostics without querying `World`, `Player`, or `Camera` directly.

#### Scenario: Snapshot is available after frame progression
- **WHEN** the overworld runtime initializes or updates the active gameplay frame
- **THEN** it publishes a render snapshot for that frame
- **AND** it publishes debug snapshot data for that frame
- **AND** both snapshots can be consumed by the outer game shell without querying `World`, `Player`, or `Camera` directly

### Requirement: Debug snapshot describes overlay-facing runtime values
The overworld runtime SHALL publish debug snapshot data containing the current player tile coordinates, active zoom percentage, retained chunk count, retained generated-content count for loaded chunks, visible tile count for the active frame, and visible generated-content count for the active frame.

#### Scenario: Overlay reads gameplay-derived diagnostics
- **WHEN** the outer game shell prepares to draw the debug overlay for the current frame
- **THEN** it can read player tile coordinates, zoom percentage, retained chunk count, retained generated-content count, visible tile count, and visible generated-content count from repo-native runtime snapshot data
- **AND** it does not need to inspect gameplay-owned modules directly to derive those values

#### Scenario: Snapshot metrics reflect bounded chunk retention
- **WHEN** the overworld runtime advances through exploration that causes old chunks to unload
- **THEN** the published retained chunk count and retained generated-content count describe only the chunks currently retained in the world cache
- **AND** those counts are allowed to decrease after retention pruning removes out-of-window chunks

#### Scenario: Snapshot visibility metrics reflect the active camera frame
- **WHEN** the overworld runtime prepares the current frame for rendering
- **THEN** the published visible tile count matches the number of visible tiles in the active render snapshot
- **AND** the published visible generated-content count matches the number of visible generated-content entries in the active render snapshot

### Requirement: Render snapshots describe visible frame content
The render snapshot SHALL include the active camera frame, the visible terrain tiles for that frame, a collection of renderable markers for gameplay entities that should appear in the frame, and a collection of renderable generated-content entries for deterministic world content visible in that frame. Snapshot-published object entries SHALL carry the world-space information required to render vegetation and gameplay markers in deterministic y-sorted order.

#### Scenario: Snapshot carries terrain, marker, generated-content, and ordering data
- **WHEN** the overworld runtime prepares the current frame for rendering
- **THEN** the snapshot includes visible tile entries covering the active camera-bounded terrain view
- **AND** it includes marker entries for the player and any other renderable entities that belong in the frame
- **AND** it includes generated-content entries for deterministic world content visible in that frame
- **AND** the published object data includes ordering information sufficient for deterministic y-sorted rendering
- **AND** it includes the camera frame used to render the snapshot

### Requirement: Snapshot entries are render-ready without gameplay reinterpretation
Each visible tile entry, marker entry, and generated-content entry in the render snapshot SHALL provide the world-space geometry and presentation identifiers needed for drawing so the outer game shell can render the frame without re-deriving gameplay presentation rules from simulation state. Player-facing snapshot data SHALL include the placement and animation selectors needed to choose the correct player sprite frame from the bundled walking spritesheet without requiring the game shell to infer facing or movement presentation from raw gameplay state. Generated-content-facing snapshot data SHALL include the identity, footprint, opaque appearance selectors, and ordering data needed to draw deterministic world content without requiring the shell to inspect `World` or raw chunk-content records directly.

#### Scenario: Game renders directly from snapshot entries
- **WHEN** the outer game shell renders an overworld frame from the published snapshot
- **THEN** it uses the snapshot's tile entries, marker entries, and generated-content entries as the source of draw data
- **AND** it does not infer additional overworld presentation state by inspecting gameplay-owned modules
- **AND** it can choose the correct player spritesheet frame from snapshot-provided player presentation metadata
- **AND** it can draw deterministic generated content from snapshot-provided content presentation metadata
- **AND** it can order vegetation and player presentation from snapshot-provided world-space ordering data

### Requirement: Camera frame sizing matches the active window dimensions
The overworld runtime SHALL publish render snapshots whose camera-frame size is derived from the active runtime viewport dimensions for the current frame so that overworld rendering stays aligned with the current window size.

#### Scenario: Snapshot camera frame reflects resized viewport
- **WHEN** the active window dimensions change before an overworld frame is updated
- **THEN** the published render snapshot uses a camera-frame size calculated from the updated viewport dimensions for that frame
- **AND** the outer game shell can render the overworld without stretching content because of stale camera-frame sizing
