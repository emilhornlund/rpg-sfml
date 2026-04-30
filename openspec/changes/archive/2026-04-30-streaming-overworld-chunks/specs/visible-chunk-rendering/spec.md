## MODIFIED Requirements

### Requirement: World provides camera-bounded terrain traversal
The world-owned rendering boundary SHALL derive visible terrain traversal from the active camera frame and enumerate only terrain that intersects the visible region plus bounded overscan. When the visible region reaches chunks that are not yet retained, the world-owned rendering boundary SHALL ensure those chunks are generated before traversing their visible tiles.

#### Scenario: Visible traversal excludes off-screen terrain
- **WHEN** the runtime requests terrain for a frame with a camera that views only a subset of the overworld
- **THEN** the world-owned rendering boundary enumerates only chunks and tiles that intersect that visible region plus bounded overscan
- **AND** it does not require traversal across a full preconfigured world rectangle for that frame

#### Scenario: Visible traversal generates newly reached chunks
- **WHEN** the active camera frame intersects chunks that have not yet been retained by the world
- **THEN** the world-owned rendering boundary generates those chunks on demand
- **AND** it enumerates visible tiles from the newly retained chunk data
