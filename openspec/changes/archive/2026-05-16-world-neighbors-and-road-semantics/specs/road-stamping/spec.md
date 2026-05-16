## MODIFIED Requirements

### Requirement: Stamping applies explicit width and intersection rules
The road-stamping stage SHALL construct tile-space road structure by applying deterministic width, shoulder, curve, endpoint, and intersection rules from segment and node metadata rather than from simple corridor overlap. Segment semantic class SHALL influence corridor treatment so trails, roads, and main roads can stamp different structural footprints. Node topology semantics SHALL influence local junction treatment so dead ends, corners, tees, crosses, and plazas stamp readable structural shapes. The stamped result SHALL preserve road continuity through bends, junctions, loops, and destination approaches while remaining suitable for world queries and renderer-facing neighbor analysis.

#### Scenario: Segment metadata controls road width
- **WHEN** a stamped road segment carries width or shoulder metadata
- **THEN** the stamped road field expands the segment into tile-space structure according to those rules
- **AND** wider or narrower segments remain deterministic for the same segment metadata and world seed

#### Scenario: Segment semantic class affects stamped structure
- **WHEN** two deterministic road segments differ in semantic class such as trail versus main road
- **THEN** the stamping stage may produce different corridor treatment for those segments
- **AND** that difference comes from deterministic structural rules rather than renderer-specific atlas data

#### Scenario: Junction stamping resolves intersecting semantics
- **WHEN** multiple stamped road segments meet at a deterministic dead end, corner, tee, cross, or plaza
- **THEN** the stamped road field resolves an explicit structural local shape instead of relying on accidental footprint overlap
- **AND** downstream systems can derive connected road motifs from that stamped result without graph-specific art metadata

### Requirement: World systems share the stamped road source of truth
World-owned systems SHALL use the stamped road field as the shared source of truth for road-covered tile queries that depend on structural road shape. Any system that excludes content, publishes visible road overlays, or analyzes local road connectivity SHALL remain aligned on the same stamped result for the same tile coordinates, including structural neighbor context exported through render snapshots for viewport-edge rendering.

#### Scenario: Content and road publication stay aligned
- **WHEN** visible road overlay publication and road-aware content exclusion evaluate the same tile coordinates
- **THEN** they consult the same stamped road result
- **AND** a tile excluded from content because of stamped road coverage matches a tile the runtime can also publish as a visible road overlay

#### Scenario: Snapshot neighbor context matches stamped structure
- **WHEN** the overworld runtime publishes visible road overlay entries for rendering
- **THEN** the neighbor occupancy exported with those entries reflects the same stamped road decisions used by world queries
- **AND** road rendering at viewport edges remains aligned with the world-owned stamped field
