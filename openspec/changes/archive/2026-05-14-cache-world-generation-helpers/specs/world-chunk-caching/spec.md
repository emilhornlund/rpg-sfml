## MODIFIED Requirements

### Requirement: Generated chunks are retained for reuse
The world-owned terrain system SHALL generate chunk data on demand the first time a query reaches a missing chunk and SHALL retain generated chunk data after first generation so repeated tile queries, metadata queries, content queries, and visible rendering queries in the same chunk can reuse the retained chunk instead of regenerating that chunk each time. `World` SHALL own the retained chunk lifecycle even when dedicated generation collaborators supply the chunk content. `World` SHALL also retain the deterministic generation collaborators it uses for missing chunk resolution for the lifetime of the world instance instead of reconstructing those collaborators on each chunk miss. The retained chunk lifecycle SHALL be bounded by an active retention window so chunks outside that window can unload while preserving deterministic regeneration when queried again later.

#### Scenario: First query generates and retains a missing chunk
- **WHEN** the runtime queries a tile, metadata record, or content record from a chunk that is not yet retained by the world
- **THEN** the world-owned terrain system generates that chunk before serving the lookup
- **AND** it retains the generated tiles, chunk metadata, and chunk content for later reuse

#### Scenario: Repeated queries reuse an existing chunk inside the retention window
- **WHEN** the runtime queries multiple tiles, metadata records, or content records from a chunk that is already retained and still falls inside the active retention window
- **THEN** the world-owned terrain system serves those lookups from the retained chunk data
- **AND** those reads do not require a second generation pass

#### Scenario: Missing chunks reuse retained generation collaborators
- **WHEN** the runtime resolves multiple distinct missing chunks through the same `World` instance
- **THEN** the world-owned terrain system reuses the same world-owned deterministic generation collaborators for those chunk loads
- **AND** it does not reconstruct those collaborators for each missing chunk before generating retained chunk data

#### Scenario: Visible render queries retain the active streaming window
- **WHEN** the runtime enumerates visible terrain or visible generated content for the active camera frame
- **THEN** the world-owned terrain system retains the chunks needed for that frame and its required visibility overscan
- **AND** nearby repeated render traversals can reuse the retained chunk data without a second generation pass

#### Scenario: Chunks outside the retention window can unload and regenerate deterministically
- **WHEN** a retained chunk falls outside the active retention window and is later queried again
- **THEN** the world-owned terrain system may unload that chunk from retained storage before the later query
- **AND** the later query regenerates the same tiles, metadata summaries, and chunk content for that chunk from deterministic generation inputs
