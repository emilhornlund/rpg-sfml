## Context

The current overworld stack already uses chunk coordinates internally, but the public `World` behavior is still defined as a finite rectangular slice. `World` prepopulates all chunks for the configured width and height, visible terrain traversal clips to those bounds, and terrain classification forces water at the world border. As a result, moving past the initial slice can never reveal newly generated land because both the runtime contract and the terrain generator treat anything outside the starting rectangle as non-world space.

This change shifts the architecture from a finite-slice cache to a streaming overworld cache. The runtime should still get deterministic terrain, stable spawn placement, and camera-bounded rendering, but the source of truth becomes absolute tile coordinates plus the world seed rather than a precomputed map extent.

## Goals / Non-Goals

**Goals:**
- Make overworld terrain queries deterministic for arbitrary absolute tile coordinates, including chunks beyond the initial player vicinity.
- Change `World` into an on-demand chunk cache that generates missing chunks during terrain lookup and visible traversal.
- Preserve the existing module boundaries where `World` owns world data, `Player` consumes traversability checks, and `Camera` remains a framing module.
- Keep the first implementation simple by making chunk loading synchronous and deterministic.

**Non-Goals:**
- Introducing background chunk generation, threading, or persistence to disk.
- Adding chunk eviction policies in the first pass.
- Expanding gameplay systems beyond terrain lookup, spawn placement, movement, and visible rendering.
- Reworking camera behavior beyond the terrain-serving contract it depends on.

## Decisions

### Decision: Treat world configuration as generation inputs, not map bounds

`WorldConfig` should continue to carry deterministic generation inputs such as seed and tile size, but finite width and height will no longer define the playable world edge for the streaming overworld path. The architecture should stop using size fields as authoritative terrain bounds.

**Why:** A streaming world cannot rely on predeclared extents without recreating the current edge-water behavior.

**Alternatives considered:**
- **Keep finite width and height and generate extra chunks outside them:** rejected because the boundary semantics remain ambiguous and tests would continue to depend on a synthetic edge.
- **Introduce a separate infinite-world config type immediately:** rejected for now because it adds API churn before the runtime behavior is proven.

### Decision: Generate terrain from absolute coordinates only

Terrain classification should depend on the seed and absolute tile coordinates, with large-scale signals providing oceans, coasts, and inland biome transitions. Border-water rules and edge-falloff based on configured world size should be removed from the streaming overworld path.

**Why:** Deterministic streaming requires each chunk to be independently derivable from coordinate space without knowing a global rectangle.

**Alternatives considered:**
- **Lazy-load the existing finite generator:** rejected because the current generator intentionally turns out-of-bounds space into water.
- **Store a full generated map and page it in/out:** rejected because it reintroduces finite-map assumptions and unnecessary memory coupling.

### Decision: Load chunks on demand inside `World`

`World` should own a retained map from chunk key to generated chunk data and expose helpers that ensure a chunk exists before tile or render queries read it.

**Why:** This keeps chunk lifecycle responsibility inside the world boundary and avoids pushing generation decisions into rendering or player movement code.

**Alternatives considered:**
- **Have `Game` orchestrate chunk loading:** rejected because it leaks world storage concerns into the top-level runtime coordinator.
- **Have the terrain generator manage global caches:** rejected because it blurs pure generation with runtime-owned retention.

### Decision: Make spawn origin-based instead of center-of-map based

Spawn selection should search for a traversable tile around a deterministic anchor near the world origin, rather than scanning outward from the center of a finite rectangle.

**Why:** A streaming overworld has no meaningful map center, but the runtime still needs deterministic initial placement.

**Alternatives considered:**
- **Hard-code the spawn tile to the origin:** rejected because the origin may not be traversable for every seed.
- **Search the first generated visible chunk only:** rejected because viewport-dependent spawn behavior is harder to reason about and test.

## Risks / Trade-offs

- **Unbounded chunk retention increases memory use over long sessions** → Mitigation: keep eviction explicitly out of scope for the first implementation, but preserve a chunk-map abstraction that can support eviction later.
- **Removing border-water rules changes existing terrain expectations and tests** → Mitigation: update specs and tests together so the new contract is explicit and deterministic.
- **Spawn search near the origin can fail for unusual seeds if the search radius is too small** → Mitigation: define deterministic outward search rules with a bounded fallback strategy.
- **Synchronous chunk generation may cause frame hitches when entering new areas** → Mitigation: keep chunk generation lightweight now and treat async prefetching as a later architectural change.

## Migration Plan

1. Update the overworld, chunk-caching, and visible-rendering specs to describe streaming behavior.
2. Refactor terrain generation to remove finite-edge assumptions and classify tiles from absolute coordinates.
3. Refactor `World` to ensure chunks on demand for tile queries and visible traversal.
4. Update spawn logic and tests to use origin-anchored streaming expectations.
5. Keep runtime callers on the existing `World`, `Player`, and `Camera` boundaries so the architectural shift remains local to world ownership.

## Open Questions

- Should the first streaming version keep width and height fields as deprecated compatibility inputs, or remove their behavioral meaning immediately?
- Do we want to reserve a minimum safe spawn biome near the origin, or is “first traversable tile in deterministic search order” sufficient?
- When chunk eviction is added later, should visible traversal drive prefetching one ring beyond the overscan area or should that remain a separate optimization?
