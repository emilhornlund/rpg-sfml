## MODIFIED Requirements

### Requirement: Retained chunks expose deterministic metadata summaries
The world-owned chunk generation system SHALL derive deterministic metadata for each generated chunk alongside the chunk's tile classifications. Each retained chunk metadata record SHALL describe the chunk's biome character and summarize its traversable space without embedding gameplay-facing candidate locations or other chunk-content records.

#### Scenario: Metadata is stable for the same chunk
- **WHEN** the runtime resolves the same chunk multiple times with the same world generation inputs
- **THEN** the retained chunk reports the same biome summary and traversability summary each time

#### Scenario: Distinct worlds agree on chunk metadata
- **WHEN** two world instances use the same generation inputs and resolve the same chunk coordinates
- **THEN** both worlds report the same metadata summary for that chunk

## REMOVED Requirements

### Requirement: Candidate locations reference traversable terrain only
**Reason**: Gameplay-facing chunk placement hints move to the dedicated `WorldContent` boundary, so chunk metadata stays focused on coarse terrain summaries.
**Migration**: Read deterministic chunk content records through the world-facing content query boundary instead of inspecting candidate locations from `ChunkMetadata`.
