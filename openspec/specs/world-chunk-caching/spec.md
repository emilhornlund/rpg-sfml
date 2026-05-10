## Purpose

Define deterministic chunk-addressed terrain lookup and chunk retention rules for the world-owned overworld cache.

## Requirements

### Requirement: World terrain queries use deterministic chunk coordinates
The world-owned terrain system SHALL resolve tile classification, retained chunk metadata, and retained chunk content through deterministic chunk coordinates and chunk-local tile coordinates derived from absolute tile coordinates instead of requiring one eagerly generated full-world tile array. The content for a missing chunk SHALL be derived from generation inputs and chunk coordinates through dedicated generation collaborators rather than by embedding generation policy directly in `World`.

#### Scenario: Stable lookup across repeated chunk-backed queries
- **WHEN** the runtime queries the same absolute tile coordinate multiple times through the world-owned terrain system
- **THEN** each query returns the same tile classification
- **AND** the chunk that owns that coordinate retains the same metadata summary across repeated access

#### Scenario: Stable lookup across distinct world instances
- **WHEN** two world instances use the same generation inputs and query the same absolute tile coordinate
- **THEN** both worlds return the same tile classification for that coordinate
- **AND** both retain the same metadata summary for the owning chunk

#### Scenario: Missing chunk content comes from deterministic generation collaborators
- **WHEN** the runtime queries a tile, metadata record, or content record in a chunk that has not yet been retained by the world
- **THEN** the world resolves that chunk's tile content, metadata, and chunk content through dedicated deterministic generation collaborators using the chunk coordinates and world generation inputs
- **AND** the resulting chunk-backed data matches later queries for the same chunk

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
The world-owned terrain system SHALL generate chunk data on demand the first time a query reaches a missing chunk and SHALL retain generated chunk data after first generation so repeated tile queries, metadata queries, content queries, and visible rendering queries in the same chunk can reuse the retained chunk instead of regenerating that chunk each time. `World` SHALL own the retained chunk lifecycle even when dedicated generation collaborators supply the chunk content. The retained chunk lifecycle SHALL be bounded by an active retention window so chunks outside that window can unload while preserving deterministic regeneration when queried again later.

#### Scenario: First query generates and retains a missing chunk
- **WHEN** the runtime queries a tile, metadata record, or content record from a chunk that is not yet retained by the world
- **THEN** the world-owned terrain system generates that chunk before serving the lookup
- **AND** it retains the generated tiles, chunk metadata, and chunk content for later reuse

#### Scenario: Repeated queries reuse an existing chunk inside the retention window
- **WHEN** the runtime queries multiple tiles, metadata records, or content records from a chunk that is already retained and still falls inside the active retention window
- **THEN** the world-owned terrain system serves those lookups from the retained chunk data
- **AND** those reads do not require a second generation pass

#### Scenario: Visible render queries retain the active streaming window
- **WHEN** the runtime enumerates visible terrain or visible generated content for the active camera frame
- **THEN** the world-owned terrain system retains the chunks needed for that frame and its required visibility overscan
- **AND** nearby repeated render traversals can reuse the retained chunk data without a second generation pass

#### Scenario: Chunks outside the retention window can unload and regenerate deterministically
- **WHEN** a retained chunk falls outside the active retention window and is later queried again
- **THEN** the world-owned terrain system may unload that chunk from retained storage before the later query
- **AND** the later query regenerates the same tiles, metadata summaries, and chunk content for that chunk from deterministic generation inputs
