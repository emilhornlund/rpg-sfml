## MODIFIED Requirements

### Requirement: World exposes metadata through a world-facing query boundary
The `World` module SHALL expose chunk metadata through world-facing queries so future gameplay systems can retrieve retained chunk summaries without accessing chunk storage directly. The world-facing metadata query boundary SHALL provide a borrowed read-only inspection path for retained metadata and SHALL document that borrowed metadata remains valid only while the addressed chunk stays retained by the owning `World` instance. The world-facing boundary SHALL also provide an owned snapshot path for callers that need metadata independent of retained chunk lifetime.

#### Scenario: Metadata query generates and retains a missing chunk
- **WHEN** a future gameplay system requests metadata for a chunk the world has not yet retained
- **THEN** the world generates and retains that chunk before returning the metadata
- **AND** later tile or metadata queries reuse the same retained chunk data

#### Scenario: Borrowed metadata inspection avoids metadata copies
- **WHEN** a caller only inspects retained chunk metadata through the borrowed world-facing query path
- **THEN** the query returns read-only access to the retained metadata without creating an owned metadata copy

#### Scenario: Owned metadata snapshot remains available beyond retained lifetime
- **WHEN** a caller needs to keep chunk metadata after the owning chunk may be evicted from retained storage
- **THEN** the world-facing query boundary provides an owned snapshot path for that metadata
- **AND** that snapshot remains usable independently of later retained-chunk eviction

#### Scenario: Borrowed metadata inspection expires with retained chunk lifetime
- **WHEN** a caller holds a borrowed metadata inspection result and the addressed chunk is later evicted from retained storage
- **THEN** the borrowed result is no longer valid for use after that eviction point
- **AND** the API documentation identifies retained chunk eviction and `World` destruction as the validity boundary
