## Context

`World` currently owns a single generated tile array built eagerly from `WorldConfig` dimensions. That approach works for the current vertical slice, but it couples terrain generation, spawn selection, runtime queries, and rendering assumptions to one finite precomputed rectangle. The codebase already has the right architectural seam for growth: `World` is the gameplay-facing boundary, while `WorldTerrainGenerator` is the world-owned collaborator for deterministic terrain construction.

This change is the first step toward a larger procedural world, but it intentionally stops short of fully removing finite-world assumptions from the runtime. The design needs to introduce chunk coordinates and cached chunk generation behind `World` without pushing chunk concerns into `Game`, `Player`, or `Camera`, and without forcing the whole runtime to adopt infinite-world rendering or camera behavior in the same step.

## Goals / Non-Goals

**Goals:**
- Move world terrain storage from one eagerly generated tile array to chunk-addressed generated data retained behind `World`.
- Define deterministic chunk coordinate math that works for absolute tile coordinates, including negative positions.
- Keep `World` as the only gameplay-facing boundary for traversability, tile lookup, spawn access, and tile/world coordinate conversion.
- Preserve the current overworld slice behavior while making future larger-world steps incremental.
- Keep terrain-generation logic in world-owned collaborators rather than growing `Game` or other gameplay modules.

**Non-Goals:**
- Making the runtime fully unbounded in this change.
- Reworking camera behavior to support unconstrained world framing.
- Replacing full-world render loops with visible-chunk rendering.
- Adding persistence, eviction policies, or streaming infrastructure beyond an in-memory chunk cache.
- Exposing chunk-management APIs broadly across gameplay modules unless needed to preserve the `World` boundary.

## Decisions

### Decision: `World` remains the public boundary and owns chunk caching internally

`World` will continue to answer gameplay-facing questions such as tile lookup, traversability, spawn access, and coordinate conversion. Chunk generation and cache storage stay behind that type, either in `World` state or dedicated world-owned helpers.

**Why:** This matches the current module architecture and keeps chunking from leaking into `Game`, `Player`, and `Camera`.

**Alternatives considered:**
- Expose chunk concepts directly to `Game`: rejected because it breaks the current ownership boundary and spreads world logic outward too early.
- Keep the eager full-world array and add chunking later: rejected because it delays the core seam this change is meant to establish.

### Decision: Chunk generation uses absolute tile coordinates, not normalized finite-map coordinates

Terrain classification will be refactored so chunk generation samples deterministic signals from absolute tile coordinates and chunk-local indexing instead of relying on one finite array pass over `widthInTiles` and `heightInTiles`.

**Why:** A cache of chunks only becomes a real step toward a larger procedural world if the generator can classify tiles from global coordinates without needing the entire map dimensions up front.

**Alternatives considered:**
- Generate finite chunks by slicing the existing normalized finite map: rejected because it preserves the existing fixed-rectangle assumption under a chunk wrapper.
- Precompute a much larger hidden finite world: rejected because it increases memory use without changing the architectural limit.

### Decision: Chunk coordinate math uses floor-division semantics

Mapping from tile coordinates to chunk coordinates and local chunk offsets will treat negative tile coordinates consistently through floor division and matching modulo logic.

**Why:** Larger procedural worlds will eventually need tiles left/up of the origin. C++ integer division truncates toward zero, which would misclassify tiles around negative chunk boundaries.

**Alternatives considered:**
- Support only non-negative chunk coordinates for now: rejected because it bakes in an avoidable asymmetry that would require later migration.
- Offset all coordinates into positive space: rejected because it hides the math and complicates reasoning about world positions.

### Decision: Spawn selection stays world-owned and uses chunk-backed traversal queries

Spawn selection will remain a `World` responsibility, but it will resolve candidate traversability through chunk-backed tile queries rather than through a fully generated finite tile array.

**Why:** Spawn remains part of the existing world-facing contract, and keeping it inside `World` avoids leaking generation details into callers.

**Alternatives considered:**
- Defer spawn until a later infinite-world change: rejected because the current runtime still depends on deterministic spawn availability.
- Keep a separate eagerly generated area only for spawn: rejected because it duplicates generation pathways.

## Risks / Trade-offs

- **Generator parity risk** -> Refactoring from finite normalized coordinates to absolute coordinate sampling can subtly change biome shapes. Mitigate with deterministic comparison tests focused on stable chunk-backed lookup behavior rather than exact preservation of every previous tile layout.
- **Boundary mismatch risk** -> `Game` and `Camera` still assume a finite world in places like `getWorldSize()` and render loops. Mitigate by keeping this change scoped to world internals and preserving existing outward behavior where required.
- **Cache correctness risk** -> Incorrect chunk keying or local-coordinate mapping could return wrong tiles at chunk edges, especially for negative coordinates. Mitigate with focused tests around chunk boundaries and repeated queries.
- **Performance ambiguity** -> Caching improves repeated lookups but introduces on-demand generation costs for the first access to a chunk. Mitigate by keeping chunk generation deterministic and coarse enough to amortize repeated gameplay queries.
