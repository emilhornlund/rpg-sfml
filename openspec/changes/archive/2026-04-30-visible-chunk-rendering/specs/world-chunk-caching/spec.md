## MODIFIED Requirements

### Requirement: Generated chunks are retained for reuse
The world-owned terrain system SHALL retain generated chunk data after first generation so repeated tile queries and visible rendering queries in the same chunk can reuse the retained chunk instead of regenerating that chunk each time.

#### Scenario: Repeated queries reuse an existing chunk
- **WHEN** the runtime queries multiple tiles from a chunk that has already been generated
- **THEN** the world-owned terrain system serves those lookups from the retained chunk data
- **AND** it does not require a second generation pass for that chunk

#### Scenario: Visible render queries reuse retained chunks
- **WHEN** the runtime enumerates visible terrain from a camera frame that intersects chunks already retained by the world
- **THEN** the world-owned terrain system serves that render traversal from the retained chunk data
- **AND** it does not require a second generation pass for those chunks
