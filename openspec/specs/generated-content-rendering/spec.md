## Purpose

Define how deterministic generated world content is exposed, published, and rendered as part of the overworld frame.

## Requirements

### Requirement: World enumerates visible generated content
The world-owned overworld system SHALL expose visible generated content for an active camera frame through a world-facing query built on retained chunk content, so runtime frame assembly can consume deterministic visible content without traversing chunk storage directly.

#### Scenario: Visible generated content comes from retained chunk content
- **WHEN** the runtime requests generated content for an active camera frame
- **THEN** the world returns generated content instances from chunks intersecting that frame
- **AND** those instances are derived from retained chunk content rather than a second generation path

#### Scenario: Content visibility respects footprint intersection
- **WHEN** a generated content instance footprint intersects the active camera frame
- **THEN** the world includes that instance in the visible generated-content query
- **AND** instances whose footprints do not intersect the frame are excluded

### Requirement: Runtime publishes render-facing generated content entries
The overworld runtime SHALL translate visible generated content into render-facing snapshot entries that preserve deterministic identity, world-space placement, footprint geometry, and opaque appearance selection without exposing SFML-specific types.

#### Scenario: Snapshot entries carry content render identifiers
- **WHEN** the overworld runtime publishes visible generated content for the active frame
- **THEN** each published entry includes world-space geometry derived from the retained content footprint
- **AND** it includes stable content identity and opaque appearance selection suitable for rendering
- **AND** the published entry does not require the shell to inspect raw `ChunkContent` records

### Requirement: Generated content renders as a world layer
The outer game shell SHALL render generated world content from snapshot-published data as part of the overworld world view instead of hiding generated content behind world-only queries. Terrain SHALL remain a dedicated terrain pass, while vegetation content and player presentation SHALL be rendered through y-sorted object presentation so overlap is determined from published world-space ordering data rather than a fixed "generated content below player" rule.

#### Scenario: Vegetation and player presentation render with y-sorted overlap
- **WHEN** the shell renders an overworld frame that includes visible vegetation content and the player marker
- **THEN** it draws terrain before object presentation
- **AND** it renders vegetation content and player presentation using snapshot-published ordering data in deterministic y-sorted order
- **AND** the resulting overlap allows the player to appear behind or in front of vegetation according to world-space depth
