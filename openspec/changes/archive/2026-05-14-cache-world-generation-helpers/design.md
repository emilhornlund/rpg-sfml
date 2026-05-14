## Context

`World` currently generates retained chunks on demand through `ensureChunkRetained()`. That method correctly reuses retained chunk data after a chunk has loaded once, but when a chunk miss occurs it reconstructs `detail::TerrainGenerator` and `detail::WorldContent` before generating terrain, metadata, and content for the missing chunk.

The desired change is a performance-oriented refactor, not a gameplay behavior change. Chunk tiles, metadata, content records, retention window behavior, and deterministic regeneration all need to remain stable. The main technical constraint is that `World.hpp` cannot directly embed the helper types as concrete members without addressing the include relationship where the helper headers already depend on `World.hpp`.

## Goals / Non-Goals

**Goals:**
- Retain world-generation helpers for the lifetime of a `World` instance instead of reconstructing them on each missing chunk load.
- Preserve deterministic chunk outputs and current chunk-retention behavior.
- Keep the refactor narrow and aligned with the existing `World` ownership boundary.

**Non-Goals:**
- Changing public `World` query APIs or chunk retention semantics.
- Introducing broader generation caches inside `TerrainGenerator` or `WorldContent`.
- Reworking `World` into a full private-implementation type unless that becomes necessary to satisfy the header dependency constraint.

## Decisions

### Retain deterministic helpers as world-owned state
`World` will construct the terrain and content helpers once from `WorldConfig` and reuse them for spawn selection and chunk loading. This keeps helper reuse local to the world-owned generation boundary and removes repeated helper setup from the chunk-miss path.

**Alternatives considered**
- Reconstruct helpers on each chunk miss: rejected because it preserves the avoidable hot-path work this change targets.
- Add static or process-wide helper singletons: rejected because helper lifetime should stay scoped to a single world configuration and instance.

### Preserve the existing `World` header boundary with indirection
Because the helper headers include `World.hpp`, the refactor should retain helper objects through an indirection-friendly representation rather than embedding those concrete types directly in the public header. The implementation can then define and initialize the retained helpers in `World.cpp` while keeping `World.hpp` lightweight and cycle-safe.

**Alternatives considered**
- Embed concrete helper objects directly in `World::State`: rejected because it would create a circular header dependency.
- Convert `World` to a full pimpl type like `Game`: rejected for now because it is a larger structural change than this focused refactor needs.

### Validate reuse through existing world-generation tests
The current tests already verify deterministic generation, retained chunk reuse, and deterministic regeneration after unloading. This change should extend those tests only where needed to prove helper retention without rewriting coverage around externally visible world behavior.

**Alternatives considered**
- Rely only on manual inspection: rejected because the refactor is performance-motivated and should still prove it preserves chunk-caching guarantees.

## Risks / Trade-offs

- **[Risk]** Introducing helper indirection could affect `World` copy semantics if ownership is made move-only. → **Mitigation:** Choose a retained-helper representation that fits the current type expectations, or explicitly update type coverage if the module is intended to stop copying.
- **[Risk]** The refactor could accidentally change spawn generation or chunk outputs if construction order differs from the current flow. → **Mitigation:** Reuse the same `WorldConfig` inputs and keep deterministic generation tests covering repeated and cross-instance results.
- **[Risk]** The performance gain is modest if helper construction remains lightweight. → **Mitigation:** Keep the change small and frame it as hot-path cleanup that also clarifies world-owned collaborator lifetime.

## Migration Plan

No deployment or data migration is required. The change is an internal runtime refactor that should ship behind the existing build and test workflow, and rollback is a direct revert of the world-helper retention changes if needed.

## Open Questions

No open product or API questions remain. The main implementation choice is the exact indirection used to retain the helpers while preserving the current `World` boundary.
