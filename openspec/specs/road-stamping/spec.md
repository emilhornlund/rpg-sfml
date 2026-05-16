## Purpose

Define how deterministic road topology is stamped into a shared structural road field before overlay publication and final visual selection.

## Requirements

### Requirement: World stamps structural road tiles from deterministic topology
The world runtime SHALL convert deterministic road topology into a stamped road field before visible road overlay publication or render-time autotile selection. The stamped road field SHALL be derived from stable road nodes, segments, and their structural metadata, and the same world seed plus queried tile coordinates SHALL produce the same stamped road result across repeated launches and repeated frame queries.

#### Scenario: Repeated stamping stays stable
- **WHEN** the runtime stamps the same road topology for the same world seed and tile-space query area multiple times
- **THEN** it produces the same stamped road structure on every evaluation
- **AND** the result does not depend on camera movement history, chunk retention order, or runtime random state

#### Scenario: Stamping remains separate from final visual selection
- **WHEN** the runtime stamps roads for a visible query area
- **THEN** the stamped result describes structural road information needed for downstream systems
- **AND** it does not require world-owned systems to publish atlas coordinates, texture rectangles, or renderer-specific tile roles

### Requirement: Stamping applies explicit width and intersection rules
The road-stamping stage SHALL construct tile-space road structure by applying deterministic width, shoulder, curve, endpoint, and intersection rules from segment and node metadata rather than from simple corridor overlap. The stamped result SHALL preserve road continuity through bends, junctions, loops, and destination approaches while remaining suitable for world queries and renderer-facing neighbor analysis.

#### Scenario: Segment metadata controls road width
- **WHEN** a stamped road segment carries width or shoulder metadata
- **THEN** the stamped road field expands the segment into tile-space structure according to those rules
- **AND** wider or narrower segments remain deterministic for the same segment metadata and world seed

#### Scenario: Junction stamping resolves intersecting segments
- **WHEN** multiple stamped road segments meet at a deterministic junction or endpoint
- **THEN** the stamped road field resolves an explicit structural intersection or endpoint shape instead of relying on accidental footprint overlap
- **AND** downstream systems can derive connected road motifs from that stamped result without graph-specific art metadata

### Requirement: World systems share the stamped road source of truth
World-owned systems SHALL use the stamped road field as the shared source of truth for road-covered tile queries that depend on structural road shape. Any system that excludes content, publishes visible road overlays, or analyzes local road connectivity SHALL remain aligned on the same stamped result for the same tile coordinates.

#### Scenario: Content and road publication stay aligned
- **WHEN** visible road overlay publication and road-aware content exclusion evaluate the same tile coordinates
- **THEN** they consult the same stamped road result
- **AND** a tile excluded from content because of stamped road coverage matches a tile the runtime can also publish as a visible road overlay
