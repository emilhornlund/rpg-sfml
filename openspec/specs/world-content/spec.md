## Purpose

Define deterministic chunk-scoped content records and world-facing content queries for the world-owned content system.

## Requirements

### Requirement: World derives deterministic content records per retained chunk
The world-owned content system SHALL derive chunk-scoped content records from the world generation seed, the owning chunk coordinates, and the retained chunk metadata for that chunk. Repeated resolution of the same chunk with the same generation inputs SHALL produce the same content records.

#### Scenario: Repeated queries return the same chunk content
- **WHEN** the runtime resolves content for the same chunk multiple times with the same world generation inputs
- **THEN** the chunk returns the same content records each time

#### Scenario: Distinct worlds agree on chunk content
- **WHEN** two world instances use the same generation inputs and resolve content for the same chunk coordinates
- **THEN** both worlds return the same content records for that chunk

### Requirement: World exposes content through a world-facing query boundary
The `World` module SHALL expose chunk content through world-facing queries so future gameplay systems can inspect deterministic retained content without accessing retained chunk storage or generation collaborators directly.

#### Scenario: Content query generates and retains a missing chunk
- **WHEN** a future gameplay system requests content for a chunk the world has not yet retained
- **THEN** the world generates and retains that chunk before returning the content records
- **AND** later tile, metadata, or content queries reuse the same retained chunk data

#### Scenario: Chunks without supported content can return no records
- **WHEN** a retained chunk's metadata does not support any deterministic content records
- **THEN** the world-facing content query returns an empty content set for that chunk
- **AND** the query does not invent unstable placeholder records
