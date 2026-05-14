## Purpose

Define deterministic chunk metadata summaries and world-facing metadata queries for retained overworld chunks.

## Requirements

### Requirement: Retained chunks expose deterministic metadata summaries
The world-owned chunk generation system SHALL derive deterministic metadata for each generated chunk alongside the chunk's tile classifications. Each retained chunk metadata record SHALL describe the chunk's biome character and summarize its traversable space without embedding gameplay-facing candidate locations or other chunk-content records.

#### Scenario: Metadata is stable for the same chunk
- **WHEN** the runtime resolves the same chunk multiple times with the same world generation inputs
- **THEN** the retained chunk reports the same biome summary and traversability summary each time

#### Scenario: Distinct worlds agree on chunk metadata
- **WHEN** two world instances use the same generation inputs and resolve the same chunk coordinates
- **THEN** both worlds report the same metadata summary for that chunk

### Requirement: Traversability and biome summaries stay coarse but useful
The chunk metadata SHALL remain lightweight rather than duplicating the full tile grid. It SHALL expose enough summary information for future systems to distinguish chunks with different biome character and to reason about whether a chunk is broadly traversable without requiring per-tile interpretation.

#### Scenario: Mixed land chunks expose a useful summary
- **WHEN** a generated chunk contains a mix of biome-relevant terrain types
- **THEN** the chunk metadata summarizes that mix in a deterministic coarse form
- **AND** future systems can distinguish the chunk from one dominated by a different biome profile

#### Scenario: Mostly blocked chunks expose low traversability
- **WHEN** a generated chunk contains little or no traversable terrain
- **THEN** the chunk metadata reports that limited traversability through its summary
- **AND** callers do not need to scan every tile in the chunk to detect that state

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
