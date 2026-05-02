## ADDED Requirements

### Requirement: World content remains data-only in the first pass
The world-owned content system SHALL expose retained chunk content as data-only structures. Returned content SHALL NOT include interaction behavior, trigger logic, inventory payloads, dialogue state, scripts, or other action-oriented gameplay logic in this change.

#### Scenario: Chunk content excludes interaction state
- **WHEN** a gameplay system reads retained chunk content from the world-facing query boundary
- **THEN** the returned content contains only deterministic identity, semantic type, spatial placement, footprint, and appearance data
- **AND** it does not require callers to interpret placeholder interaction fields

## MODIFIED Requirements

### Requirement: World derives deterministic content records per retained chunk
The world-owned content system SHALL derive deterministic chunk content from the world generation seed, the owning chunk coordinates, and the retained chunk metadata for that chunk. Repeated resolution of the same chunk with the same generation inputs SHALL produce the same chunk content structure, including the same content instances, semantic types, world positions, footprints, and appearance identifiers.

#### Scenario: Repeated queries return the same chunk content
- **WHEN** the runtime resolves content for the same chunk multiple times with the same world generation inputs
- **THEN** the chunk returns the same content instances each time
- **AND** each returned instance keeps the same deterministic identity, semantic type, world-space placement, footprint, and appearance selection

#### Scenario: Distinct worlds agree on chunk content
- **WHEN** two world instances use the same generation inputs and resolve content for the same chunk coordinates
- **THEN** both worlds return the same chunk content structure for that chunk
- **AND** the content instances within that chunk agree on deterministic identity, placement, footprint, and appearance data

### Requirement: World exposes content through a world-facing query boundary
The `World` module SHALL expose chunk content through world-facing queries so future gameplay systems can inspect deterministic retained content without accessing retained chunk storage or generation collaborators directly. The returned chunk content SHALL identify the owning chunk once at the chunk-content level and expose content instances as structured data entries rather than a minimal flat record list.

#### Scenario: Content query generates and retains a missing chunk
- **WHEN** a future gameplay system requests content for a chunk the world has not yet retained
- **THEN** the world generates and retains that chunk before returning the chunk content
- **AND** later tile, metadata, or content queries reuse the same retained chunk data

#### Scenario: Chunks without supported content can return no instances
- **WHEN** a retained chunk's metadata does not support any deterministic content instances
- **THEN** the world-facing content query returns an empty instance collection for that chunk
- **AND** the query does not invent unstable placeholder content
