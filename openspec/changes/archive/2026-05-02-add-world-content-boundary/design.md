## Context

`World` already owns retained chunk lifecycle and world-facing queries, while `TerrainGenerator` currently produces both chunk tiles and chunk metadata. That metadata contract includes candidate locations, which are really gameplay-facing placement hints rather than coarse terrain summaries. The proposed change introduces a dedicated `WorldContent` collaborator so deterministic per-chunk content can grow behind the world boundary without expanding terrain generation responsibilities.

The codebase already has the right retention seam in `World::ensureChunkRetained`, where generated chunk data is produced once and cached for reuse. This change should preserve that seam, keep `OverworldRuntime` unaware of content-generation details, and keep `World` as the only gameplay-facing access point for chunk-owned data.

## Goals / Non-Goals

**Goals:**
- Add a dedicated world-owned boundary for deterministic chunk content generation.
- Keep `ChunkMetadata` focused on coarse terrain summaries instead of gameplay-facing candidate placement.
- Retain chunk content alongside tiles and metadata so repeated queries reuse the same retained chunk data.
- Expose world-facing content queries through `World` rather than through generator helpers or runtime orchestration code.

**Non-Goals:**
- Defining rich gameplay systems that consume the new content records.
- Changing how visible terrain rendering, player movement, or camera behavior work.
- Replacing chunk-based terrain generation or moving chunk retention ownership out of `World`.

## Decisions

### `WorldContent` is a collaborator owned by `World`

`WorldContent` should be a dedicated world-owned module or helper used from `World::ensureChunkRetained`, not a peer gameplay module and not a runtime concern in `OverworldRuntime`.

**Why:** the existing code already centralizes chunk retention and chunk-backed queries in `World`. Keeping content generation inside that retention path preserves the current boundary: `World` owns cached chunk state, while collaborators provide deterministic generation policy.

**Alternatives considered:**
- Keep content derivation inside `TerrainGenerator`: rejected because it continues mixing terrain generation with gameplay-facing content logic.
- Generate content in `OverworldRuntime`: rejected because runtime orchestration should consume world state, not define chunk-content policy.

### `ChunkMetadata` becomes terrain-summary-only

The current candidate list in `ChunkMetadata` should be removed. Metadata should keep biome and traversability summaries only, with any future placement interpretation moving into `WorldContent`.

**Why:** candidate locations are already a lightweight content system. Leaving them in metadata would create overlapping responsibilities between `ChunkMetadata` and `WorldContent`.

**Alternatives considered:**
- Keep candidates in metadata and let `WorldContent` transform them: rejected because it preserves the boundary leak and weakens the value of the new module.
- Move all content-facing hints into raw tile scans inside `WorldContent`: rejected because it bypasses the metadata boundary and couples content generation back to terrain details.

### Content generation uses `seed + chunk coordinates + chunk metadata`

The `WorldContent` input contract should be the world generation seed, the owning chunk coordinates, and the retained chunk metadata for that chunk.

**Why:** this keeps content generation deterministic, cheap to recompute, and decoupled from raw tile storage. It also matches the intended boundary the team already agreed on.

**Alternatives considered:**
- Pass full chunk tiles to `WorldContent`: rejected because it dilutes the metadata boundary and makes richer content too dependent on terrain internals.
- Use world-global mutable state: rejected because determinism and replayability would become harder to reason about.

### Retained chunks store tiles, metadata, and content together

The retained chunk record inside `World` should expand so one cache entry contains all chunk-scoped retained data.

**Why:** the world cache already guarantees one-time generation and reuse. Content records should follow the same lifecycle as tiles and metadata.

**Alternatives considered:**
- Separate content cache keyed independently from chunk retention: rejected because it introduces parallel cache lifecycles for the same chunk identity.

## Risks / Trade-offs

- **Metadata may be too coarse for future content richness** -> Start with coarse deterministic content records and expand the metadata contract later only when a concrete content use case requires it.
- **Removing candidates from metadata will ripple through tests and specs** -> Update the existing metadata capability and chunk-related tests in the same change so the boundary shift stays coherent.
- **A new query surface can overlap with future gameplay systems** -> Keep the initial content record model small and chunk-scoped so later systems can build on it instead of replacing it.

## Migration Plan

1. Add the `WorldContent` module and content record types.
2. Update retained chunk storage in `World` to cache content alongside tiles and metadata.
3. Remove candidate records from `ChunkMetadata` and stop producing them in `TerrainGenerator`.
4. Add world-facing content queries and update tests/specs to validate the new boundary.

Rollback is straightforward: revert the change so chunk metadata once again carries candidates and `World` retains only tiles plus metadata.

## Open Questions

- Should the initial content record type include only chunk-local anchors and type identifiers, or also a deterministic record id for future save/load hooks?
- Do we want one content query that returns all records for a chunk, or both chunk-addressed and tile-addressed convenience queries from `World`?
