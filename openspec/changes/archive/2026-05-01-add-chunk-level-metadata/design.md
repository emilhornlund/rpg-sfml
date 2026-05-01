## Context

`World` currently retains generated chunk state as `std::map<std::pair<int, int>, std::vector<TileType>>`, and `TerrainGenerator::generateChunk()` only returns per-tile terrain classifications. That is enough for rendering, traversability checks, and the current spawn search, but it leaves future gameplay systems without any chunk-scale signals to build on. The next wave of world features needs a small amount of structure behind the `World` boundary so biome-aware spawning, encounter seeding, and point-of-interest placement do not have to rediscover meaning from raw tiles every time.

The repository already keeps world-generation policy inside `World`-owned collaborators and keeps `Game` focused on orchestration. This change should preserve those boundaries, reuse the existing chunk-addressed generation flow, and avoid introducing persistence, entity lifecycles, or a generalized data-driven runtime architecture.

## Goals / Non-Goals

**Goals:**
- Generate deterministic chunk metadata in the same world-owned flow that already produces chunk tile data.
- Retain chunk metadata next to chunk terrain so repeated queries reuse cached results.
- Provide enough metadata for future systems to reason about chunk biome character, traversability, and candidate spawn or point-of-interest locations.
- Keep the design narrow and compatible with the current `World` / `TerrainGenerator` split.

**Non-Goals:**
- Defining a persistence format for chunk state
- Introducing an ECS, simulation graph, or generalized gameplay content pipeline
- Building the actual gameplay systems that consume the metadata
- Reworking rendering or camera flow around chunk metadata

## Decisions

### 1. Retained chunk state becomes a structured chunk record

`World::State` should stop storing a bare `std::vector<TileType>` per chunk and instead retain a chunk record that contains both tile content and metadata. This keeps chunk ownership inside `World`, lets tile and metadata queries share one lifecycle, and avoids keeping parallel caches in sync.

**Why this over parallel maps?**
Parallel `tilesByChunk` and `metadataByChunk` maps would duplicate key management, increase drift risk, and make reuse rules harder to reason about.

### 2. Metadata is generated in the same pass as chunk terrain

`TerrainGenerator::generateChunk()` should return both tiles and metadata, with metadata derived while the generator already iterates the chunk's tiles. The same pass can accumulate biome counts, traversable coverage, and candidate tiles without rescanning retained terrain later.

**Why this over lazy metadata derivation?**
Lazy post-processing would either rescan tiles on first metadata access or force another helper to duplicate chunk traversal logic. Generating both outputs together keeps determinism simple and makes metadata reuse implicit.

### 3. Metadata stays coarse, chunk-scoped, and future-facing

The metadata payload should remain intentionally lightweight:
- biome summary: dominant biome classification plus enough aggregate signal to distinguish mixed chunks
- traversability summary: coarse information about how much walkable terrain the chunk contains
- candidate locations: zero or more deterministic traversable tiles tagged for future spawn or point-of-interest use

This is enough to keep chunks from being "just colored tiles" without committing to a heavier representation such as region graphs, navigation meshes, or saved world state.

**Why this over richer region data now?**
Richer structures would force early decisions about persistence, AI navigation, encounter ownership, and update lifecycles that this proposal explicitly defers.

### 4. World-facing access remains chunk-addressed

Future consumers should query chunk metadata through `World`, using chunk coordinates or world tile coordinates that resolve to chunk coordinates internally. `Game`, `Player`, and `Camera` should not interact with retained chunk records directly.

**Why this over exposing generator internals?**
Keeping metadata access behind `World` preserves the existing gameplay boundary and leaves room to evolve the metadata shape without leaking internal storage details into the runtime shell.

### 5. Spawn behavior stays behaviorally stable in this change

The metadata should include spawn or point-of-interest candidates, but the proposal does not require a gameplay-visible redesign of player spawn selection yet. The current spawn search can remain intact initially, or it can be refactored to consume the same candidate logic as long as the resulting spawn remains deterministic and traversable.

**Why not switch all spawn selection to metadata immediately?**
That would turn an infrastructure change into a gameplay change. The proposal is meant to prepare future systems first, not couple the metadata rollout to a broader spawn-design discussion.

## Risks / Trade-offs

- **[Metadata becomes too detailed too early]** → Keep the required summaries coarse and avoid fields that imply persistence or live simulation ownership.
- **[Generator and world storage drift apart]** → Use one chunk record type that is produced by the generator and retained by `World`.
- **[Candidate selection is too brittle for future gameplay needs]** → Treat candidate lists as deterministic hints, not final gameplay semantics; future systems can filter or rank them further.
- **[Existing tile-centric tests become harder to reason about]** → Preserve current tile query behavior and add focused tests around metadata determinism and reuse rather than rewriting world tests wholesale.

## Migration Plan

1. Introduce chunk metadata types and a structured retained chunk record behind the `World` boundary.
2. Update terrain generation collaborators to build metadata together with tile content for each generated chunk.
3. Extend `World` chunk retention and query helpers to reuse the structured chunk record for both tile and metadata access.
4. Add or update tests and specs for deterministic metadata generation, retention, and candidate validity.
5. Keep gameplay-visible spawn behavior unchanged unless a refactor to shared candidate logic preserves current guarantees.

Rollback is straightforward because the change is internal to world generation and retention: reverting the chunk record and metadata query surface restores the prior tile-only behavior.

## Open Questions

- Should point-of-interest candidates carry a small typed label set from the start, or is one generic candidate bucket enough for the first implementation?
- Should the traversability summary expose only coarse coverage, or also identify whether the chunk contains contiguous walkable regions near its center?
