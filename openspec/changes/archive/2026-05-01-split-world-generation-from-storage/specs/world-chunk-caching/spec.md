## MODIFIED Requirements

### Requirement: World terrain queries use deterministic chunk coordinates
The world-owned terrain system SHALL resolve tile classification through deterministic chunk coordinates and chunk-local tile coordinates derived from absolute tile coordinates instead of requiring one eagerly generated full-world tile array. The content for a missing chunk SHALL be derived from generation inputs and chunk coordinates through dedicated generation collaborators rather than by embedding generation policy directly in `World`.

#### Scenario: Stable lookup across repeated chunk-backed queries
- **WHEN** the runtime queries the same absolute tile coordinate multiple times through the world-owned terrain system
- **THEN** each query returns the same tile classification
- **AND** the result does not depend on whether nearby tiles were queried first

#### Scenario: Stable lookup across distinct world instances
- **WHEN** two world instances use the same generation inputs and query the same absolute tile coordinate
- **THEN** both worlds return the same tile classification for that coordinate

#### Scenario: Missing chunk content comes from deterministic generation collaborators
- **WHEN** the runtime queries a tile in a chunk that has not yet been retained by the world
- **THEN** the world resolves that chunk's content through dedicated deterministic generation collaborators using the chunk coordinates and world generation inputs
- **AND** the resulting tile classification matches later queries for the same coordinate

### Requirement: Generated chunks are retained for reuse
The world-owned terrain system SHALL generate chunk data on demand the first time a query reaches a missing chunk and SHALL retain generated chunk data after first generation so repeated tile queries and visible rendering queries in the same chunk can reuse the retained chunk instead of regenerating that chunk each time. `World` SHALL own the retained chunk lifecycle even when dedicated generation collaborators supply the chunk content.

#### Scenario: First query generates and retains a missing chunk
- **WHEN** the runtime queries a tile from a chunk that is not yet retained by the world
- **THEN** the world-owned terrain system generates that chunk before serving the lookup
- **AND** it retains the generated chunk for later reuse

#### Scenario: Repeated queries reuse an existing chunk
- **WHEN** the runtime queries multiple tiles from a chunk that has already been generated
- **THEN** the world-owned terrain system serves those lookups from the retained chunk data
- **AND** it does not require a second generation pass for that chunk

#### Scenario: Visible render queries reuse retained chunks
- **WHEN** the runtime enumerates visible terrain from a camera frame that intersects chunks already retained by the world
- **THEN** the world-owned terrain system serves that render traversal from the retained chunk data
- **AND** it does not require a second generation pass for those chunks
