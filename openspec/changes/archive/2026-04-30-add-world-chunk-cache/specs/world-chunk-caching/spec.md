## ADDED Requirements

### Requirement: World terrain queries use deterministic chunk coordinates
The world-owned terrain system SHALL resolve tile classification through deterministic chunk coordinates and chunk-local tile coordinates derived from absolute tile coordinates instead of requiring one eagerly generated full-world tile array.

#### Scenario: Stable lookup across repeated chunk-backed queries
- **WHEN** the runtime queries the same absolute tile coordinate multiple times through the world-owned terrain system
- **THEN** each query returns the same tile classification
- **AND** the result does not depend on whether nearby tiles were queried first

#### Scenario: Stable lookup across distinct world instances
- **WHEN** two world instances use the same generation inputs and query the same absolute tile coordinate
- **THEN** both worlds return the same tile classification for that coordinate

### Requirement: Chunk coordinate mapping handles negative tile positions
The world-owned terrain system SHALL map absolute tile coordinates to chunk coordinates and chunk-local tile coordinates using floor-division semantics so tiles on both sides of the origin resolve to the intended chunk.

#### Scenario: Tiles left of a chunk boundary resolve consistently
- **WHEN** the runtime queries tile coordinates immediately to the left of a chunk origin
- **THEN** those tiles resolve to the previous chunk rather than the chunk at the origin
- **AND** their chunk-local coordinates remain inside valid chunk-local bounds

#### Scenario: Tiles above a chunk boundary resolve consistently
- **WHEN** the runtime queries tile coordinates immediately above a chunk origin
- **THEN** those tiles resolve to the previous chunk rather than the chunk at the origin
- **AND** their chunk-local coordinates remain inside valid chunk-local bounds

### Requirement: Generated chunks are retained for reuse
The world-owned terrain system SHALL retain generated chunk data after first generation so repeated tile queries in the same chunk can reuse the retained chunk instead of regenerating that chunk each time.

#### Scenario: Repeated queries reuse an existing chunk
- **WHEN** the runtime queries multiple tiles from a chunk that has already been generated
- **THEN** the world-owned terrain system serves those lookups from the retained chunk data
- **AND** it does not require a second generation pass for that chunk
