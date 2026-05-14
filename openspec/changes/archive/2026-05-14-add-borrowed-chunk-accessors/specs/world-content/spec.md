## MODIFIED Requirements

### Requirement: World exposes content through a world-facing query boundary
The `World` module SHALL expose chunk content through world-facing queries so future gameplay systems can inspect deterministic retained content without accessing retained chunk storage or generation collaborators directly. The world-facing query boundary SHALL provide a borrowed read-only inspection path that does not require copying retained chunk content for inspection-only reads. The borrowed inspection result SHALL identify the owning chunk once at the chunk-content level and expose content instances as structured data entries rather than a minimal flat record list. The world-facing boundary SHALL also provide an owned snapshot path for callers that need chunk content independent of retained chunk lifetime.

#### Scenario: Content query generates and retains a missing chunk
- **WHEN** a future gameplay system requests content for a chunk the world has not yet retained
- **THEN** the world generates and retains that chunk before returning the chunk content
- **AND** later tile, metadata, or content queries reuse the same retained chunk data

#### Scenario: Chunks without supported content can return no instances
- **WHEN** a retained chunk's metadata does not support any deterministic content instances
- **THEN** the world-facing content query returns an empty instance collection for that chunk
- **AND** the query does not invent unstable placeholder records

#### Scenario: Borrowed content inspection avoids chunk-content copies
- **WHEN** a caller only inspects retained chunk content through the borrowed world-facing query path
- **THEN** the query returns read-only access to the retained chunk content without creating an owned chunk-content copy
- **AND** the caller can iterate the retained content instances through that borrowed result

#### Scenario: Owned snapshot content remains available beyond retained lifetime
- **WHEN** a caller needs to keep chunk content after the owning chunk may be evicted from retained storage
- **THEN** the world-facing query boundary provides an owned snapshot path for that chunk content
- **AND** that snapshot remains usable independently of later retained-chunk eviction

#### Scenario: Borrowed content inspection expires with retained chunk lifetime
- **WHEN** a caller holds a borrowed chunk-content inspection result and the addressed chunk is later evicted from retained storage
- **THEN** the borrowed result is no longer valid for use after that eviction point
- **AND** the API documentation identifies retained chunk eviction and `World` destruction as the validity boundary
