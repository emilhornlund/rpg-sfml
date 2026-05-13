## MODIFIED Requirements

### Requirement: Overworld runtime publishes render snapshots
The overworld gameplay runtime SHALL publish a render snapshot for the active frame instead of requiring the outer game shell to derive overworld presentation state ad hoc. The render snapshot SHALL be expressed through repo-native runtime types and SHALL not expose SFML-specific types. The overworld runtime SHALL also publish repo-native debug snapshot data for the active frame so the outer game shell can render diagnostics without querying `World`, `Player`, or `Camera` directly. Initialization SHALL publish an initial render/debug snapshot for the provided viewport, and each `update()` call SHALL publish the current frame snapshot through a single refresh pass after the frame's gameplay state and camera framing have been applied.

#### Scenario: Snapshot is available after frame progression
- **WHEN** the overworld runtime initializes or updates the active gameplay frame
- **THEN** it publishes a render snapshot for that frame
- **AND** it publishes debug snapshot data for that frame
- **AND** both snapshots can be consumed by the outer game shell without querying `World`, `Player`, or `Camera` directly

#### Scenario: Frame update publishes post-simulation state once
- **WHEN** the overworld runtime advances one gameplay frame through `update()`
- **THEN** the published render and debug snapshots reflect the player position, camera frame, and visible world state for that updated frame
- **AND** the runtime does not perform an extra pre-simulation snapshot refresh for the same `update()` call
