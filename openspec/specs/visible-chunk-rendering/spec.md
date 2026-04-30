## Purpose

Define camera-bounded terrain traversal rules for visible overworld rendering.

## Requirements

### Requirement: World provides camera-bounded terrain traversal
The world-owned rendering boundary SHALL derive visible terrain traversal from the active camera frame and enumerate only terrain that intersects the visible region plus bounded overscan.

#### Scenario: Visible traversal excludes off-screen terrain
- **WHEN** the runtime requests terrain for a frame with a camera that views only a subset of the overworld
- **THEN** the world-owned rendering boundary enumerates only chunks and tiles that intersect that visible region plus bounded overscan
- **AND** it does not require traversal across the full configured world dimensions for that frame

#### Scenario: Visible traversal is clipped at world edges
- **WHEN** the active camera frame touches or overlaps an overworld boundary
- **THEN** the world-owned rendering boundary clips traversal to valid world coordinates
- **AND** it does not enumerate tiles outside the generated world
