## MODIFIED Requirements

### Requirement: Overworld runtime publishes render snapshots
The overworld gameplay runtime SHALL publish a render snapshot for the active frame instead of requiring the outer game shell to derive overworld presentation state ad hoc. The render snapshot SHALL be expressed through repo-native runtime types and SHALL not expose SFML-specific types. The overworld runtime SHALL also publish repo-native debug snapshot data for the active frame so the outer game shell can render diagnostics without querying `World`, `Player`, or `Camera` directly.

#### Scenario: Snapshot is available after frame progression
- **WHEN** the overworld runtime initializes or updates the active gameplay frame
- **THEN** it publishes a render snapshot for that frame
- **AND** it publishes debug snapshot data for that frame
- **AND** both snapshots can be consumed by the outer game shell without querying `World`, `Player`, or `Camera` directly

## ADDED Requirements

### Requirement: Debug snapshot describes overlay-facing runtime values
The overworld runtime SHALL publish debug snapshot data containing the current player tile coordinates, active zoom percentage, retained generated-content count for loaded chunks, and visible generated-content count for the active frame.

#### Scenario: Overlay reads gameplay-derived diagnostics
- **WHEN** the outer game shell prepares to draw the debug overlay for the current frame
- **THEN** it can read player tile coordinates, zoom percentage, loaded generated-content count, and rendered generated-content count from repo-native runtime snapshot data
- **AND** it does not need to inspect gameplay-owned modules directly to derive those values
