## ADDED Requirements

### Requirement: Render snapshots publish visible road overlay entries
The overworld runtime SHALL publish visible road overlay entries for the active camera frame as part of the gameplay-owned render snapshot so the outer game shell can render road overlays without querying `World` directly.

#### Scenario: Snapshot carries visible road overlay geometry and surface context
- **WHEN** the overworld runtime prepares a render snapshot for a frame containing visible roads
- **THEN** the snapshot includes visible road overlay entries covering the active camera-bounded area
- **AND** each entry provides the world-space geometry and underlying terrain context needed to render the overlay tile
- **AND** the outer game shell does not need to inspect `World` directly to discover road-covered tiles for that frame

