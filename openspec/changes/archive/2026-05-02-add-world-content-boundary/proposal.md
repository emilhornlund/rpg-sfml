## Why

Chunk generation currently mixes terrain summary concerns with gameplay-facing placement hints by storing candidate locations directly in `ChunkMetadata`. That makes the terrain metadata contract do double duty and leaves no dedicated boundary for deterministic per-chunk world content that can grow without expanding `TerrainGenerator`.

## What Changes

- Add a new world-owned `WorldContent` boundary that derives deterministic content records from the world seed, chunk coordinates, and chunk metadata.
- Extend the `World` retained chunk boundary so each retained chunk can serve content records alongside tiles and metadata.
- Refocus `ChunkMetadata` on coarse terrain summaries and remove gameplay-facing candidate records from the metadata contract.
- Introduce world-facing content queries so future gameplay systems can inspect chunk content through `World` without reaching into retained chunk storage or terrain generation helpers.

## Capabilities

### New Capabilities
- `world-content`: Deterministic chunk-scoped content records generated behind a dedicated world-owned boundary and exposed through world-facing queries.

### Modified Capabilities
- `world-chunk-metadata`: Remove gameplay-facing candidate locations from chunk metadata so the metadata contract stays focused on coarse terrain summaries.
- `world-chunk-caching`: Retain deterministic content records alongside tiles and metadata for each generated chunk.
- `gameplay-modules`: Extend the world-owned collaborator boundary so `World` can delegate chunk content generation to a dedicated `WorldContent` module while preserving `World` as the gameplay-facing access point.

## Impact

- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, `src/main/WorldTerrainGenerator.*`, new world-content module files, and chunk-related tests.
- Affected behavior: chunk metadata becomes terrain-summary-only, while deterministic content records move to a new world query surface.
- Affected systems: world-owned chunk retention, future content placement hooks, and tests/specs that currently treat metadata candidates as gameplay-facing signals.
