## MODIFIED Requirements

### Requirement: World owns a deterministic road network topology
The world runtime SHALL derive a small road network from deterministic world-owned data rather than from render-time heuristics. The network SHALL contain stable road nodes and connecting segments suitable for representing spawn-adjacent topology such as junctions, endpoints, branches, loops, and optional destination anchors. Road nodes and segments SHALL also retain deterministic structural metadata needed by downstream road stamping, including corridor width and local junction treatment inputs, while remaining independent of renderer-facing atlas data. The same world seed SHALL produce the same road-network topology across repeated launches.

#### Scenario: Repeated topology generation stays stable
- **WHEN** the world builds its road network multiple times for the same world seed
- **THEN** it produces the same set of road nodes and connecting segments on every build
- **AND** the result does not depend on camera movement history, chunk retention order, or runtime random state

#### Scenario: Spawn is connected into the deterministic network
- **WHEN** the world generates the deterministic road network for a valid spawn tile
- **THEN** the network includes a road node or connected junction anchored at or adjacent to spawn
- **AND** at least one road segment connects spawn into the rest of the generated network

#### Scenario: Topology can express branches, loops, and destinations
- **WHEN** the world generates a deterministic road network for seeds that select richer layouts
- **THEN** the resulting topology may include branch junctions, loop-closing segment connections, and destination-oriented endpoints or centers
- **AND** those features remain encoded as nodes, segment connections, and deterministic structural metadata rather than as hard-coded occupancy special cases

### Requirement: Road occupancy is projected from topology footprints
The world runtime SHALL hand deterministic road topology to a dedicated road-stamping stage instead of treating topology footprints as the final published rasterized road shape. Topology-owned nodes, segments, and their structural metadata SHALL define the intent that the stamping stage expands into tile-space road structure for segments, junctions, endpoints, and destination approaches. Tiles covered by that stamped structure SHALL be treated as road-covered consistently across world queries, visible road publication, and road-aware content exclusion.

#### Scenario: Segment footprints publish road-covered tiles
- **WHEN** a tile falls within the stamped structure produced from a deterministic road segment
- **THEN** road occupancy queries report that tile as road-covered
- **AND** visible road overlay publication may include that tile when its underlying surface supports road overlays

#### Scenario: Junction and endpoint footprints remain queryable
- **WHEN** a tile falls within the stamped structure produced for a deterministic road junction, branch meeting, or endpoint
- **THEN** road occupancy queries report that tile as road-covered
- **AND** the result does not require special render-only logic outside the shared stamped road source of truth

#### Scenario: Topology-backed occupancy stays aligned across systems
- **WHEN** world-content placement and visible road overlay collection evaluate the same tile coordinates
- **THEN** they use the same topology-backed stamped road decision
- **AND** a tile excluded from content because of road coverage matches the tile that the runtime can also publish as a visible road overlay
