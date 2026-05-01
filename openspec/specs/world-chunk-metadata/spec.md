## Purpose

Define deterministic chunk metadata summaries and world-facing metadata queries for retained overworld chunks.

## Requirements

### Requirement: Retained chunks expose deterministic metadata summaries
The world-owned chunk generation system SHALL derive deterministic metadata for each generated chunk alongside the chunk's tile classifications. Each retained chunk metadata record SHALL describe the chunk's biome character, summarize its traversable space, and provide candidate locations that future spawn or point-of-interest systems can inspect without rescanning raw tiles.

#### Scenario: Metadata is stable for the same chunk
- **WHEN** the runtime resolves the same chunk multiple times with the same world generation inputs
- **THEN** the retained chunk reports the same biome summary, traversability summary, and candidate locations each time

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

### Requirement: Candidate locations reference traversable terrain only
The chunk metadata SHALL identify zero or more deterministic candidate tiles for future spawn and point-of-interest placement. Every candidate location SHALL resolve to a traversable tile, and chunks without suitable terrain MAY expose no candidates.

#### Scenario: Candidate locations stay on walkable terrain
- **WHEN** the runtime inspects candidate locations from retained chunk metadata
- **THEN** each candidate resolves to a traversable tile in the owning chunk

#### Scenario: Unusable chunks can expose no candidates
- **WHEN** a generated chunk does not contain terrain suitable for spawn or point-of-interest placement
- **THEN** the chunk metadata exposes an empty candidate set instead of inventing invalid locations

### Requirement: World exposes metadata through a world-facing query boundary
The `World` module SHALL expose chunk metadata through world-facing queries so future gameplay systems can retrieve retained chunk summaries without accessing chunk storage directly.

#### Scenario: Metadata query generates and retains a missing chunk
- **WHEN** a future gameplay system requests metadata for a chunk the world has not yet retained
- **THEN** the world generates and retains that chunk before returning the metadata
- **AND** later tile or metadata queries reuse the same retained chunk data
