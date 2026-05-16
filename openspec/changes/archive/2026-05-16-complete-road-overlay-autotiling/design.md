## Context

The current road overlay feature has two disconnected halves. World code publishes road membership as a thin one-tile-wide cross centered on the spawn tile, while render code already resolves surface-aware transition roles from neighboring road occupancy. Because the occupancy mask is too narrow and the road metadata loader only keeps `base` and `transition` overlay classes, the runtime cannot realize the authored two-to-three-tile dirt road shapes or the newly staged sparse `decor` variants.

The change touches multiple modules that currently duplicate road membership logic: world visibility, world-content placement exclusions, road overlay metadata loading, and road batch rendering. The updated behavior must stay deterministic for the same world seed and coordinates, reuse the staged catalog as the source of truth, and avoid introducing gameplay-facing road tile types.

## Goals / Non-Goals

**Goals:**
- Publish deterministic road occupancy that produces roads at least two tiles wide and allows deterministic three-tile widening segments.
- Separate road overlay metadata into base, decor, and transition pools so interior road rendering can choose sparse decor replacements without affecting transition tiles.
- Keep road rendering neighbor-driven and surface-aware so existing transition tiles can be composed into edges, corners, widened shapes, and end-cap motifs.
- Keep world-content exclusion aligned with the widened road footprint so vegetation does not regrow through visible roads.

**Non-Goals:**
- Introducing a new gameplay-facing road terrain type or changing traversability rules.
- Adding new overlay atlas roles or requiring changes to vendored rendering dependencies.
- Reworking the overall road network layout beyond the current deterministic spawn-centered road structure.

## Decisions

### Decision: Represent roads as deterministic occupancy around a centerline band
Road membership will continue to be derived from deterministic world data, but the occupancy test should move from `deltaX == 0` / `deltaY == 0` to a centerline-plus-width model. Horizontal and vertical road segments remain anchored at spawn, with a minimum half-width that yields two-tile roads and deterministic widening windows that temporarily extend the road to three tiles.

This keeps the current world-facing contract simple: road presence is still a boolean query over coordinates. It also lets the renderer infer transitions from local occupancy rather than needing a second authored road-shape graph.

Alternatives considered:
- **Store explicit precomputed road tiles per chunk**: rejected because the current behavior does not require persistent chunk data and would duplicate deterministic logic.
- **Publish a gameplay road tile type**: rejected because roads are still a render/detail concern layered over existing terrain.

### Decision: Mirror terrain appearance selection for road interiors
Road overlay metadata should store separate `base` and `decor` variant pools, with deterministic selection for interior road tiles using a sparse decor roll plus a stable variant hash. Transition tiles remain authoritative whenever a road tile resolves to an edge or corner role; only non-transition interiors participate in base-versus-decor selection.

This mirrors the proven terrain autotile pattern and directly matches the updated catalog semantics. It also avoids visual noise on road borders, where decor replacements could break the legibility of the transition silhouettes.

Alternatives considered:
- **Treat decor as additional base variants with lower weights**: rejected because it erases the authored semantic split and makes sparse placement harder to reason about in tests.
- **Allow decor on transition tiles**: rejected because surface transitions need to stay visually crisp and role-driven.

### Decision: Support end caps and widened motifs through occupancy-aware composition
The renderer should continue resolving one atlas cell per occupied road tile from neighboring road occupancy, but the widened occupancy patterns must be authored so existing transition roles can compose into motifs such as horizontal and vertical end caps. The design assumes end-cap appearances emerge from adjacent road tiles resolving to coordinated top/bottom/left/right corner roles, rather than from introducing special single-tile cap assets.

This preserves the current batched rendering model and leverages the staged tileset as-is. The road shape generator therefore has to be "creative" in how it produces local occupancy, because that occupancy is the language the renderer uses to compose motifs.

Alternatives considered:
- **Introduce dedicated end-cap roles in the loader and atlas**: rejected because the existing tileset can already express caps compositionally.
- **Add a multi-tile road motif renderer separate from the batch path**: rejected because it adds a second rendering system before the current occupancy-driven one is exhausted.

### Decision: Keep duplicated road occupancy logic in sync through a shared helper
`World.cpp` and `WorldContent.cpp` currently duplicate the same road-presence calculation. The widened occupancy rules should live in one shared helper boundary consumed by both sites so visible roads and vegetation exclusion cannot diverge.

Alternatives considered:
- **Leave the duplication and update both call sites manually**: rejected because the new width and widening logic is more complex than the current line test and would be easy to desynchronize.

## Risks / Trade-offs

- **[Risk] Widened roads produce neighbor patterns the current role resolver does not map cleanly** → Mitigation: update road-shape rules and renderer tests together, and explicitly cover caps, edges, corners, and widened interiors in batch-selection tests.
- **[Risk] Sparse decor makes roads look noisy or overgrown** → Mitigation: use a deterministic low-frequency decor roll similar to terrain and keep decor selection limited to non-transition interior tiles.
- **[Risk] World-content exclusion no longer matches visible road occupancy** → Mitigation: route both world visibility and world-content placement through the same shared road-occupancy helper.
- **[Risk] Existing metadata tests encode obsolete assumptions about base counts** → Mitigation: rewrite tests to assert separate base, decor, and transition expectations instead of a single flat base pool.

## Migration Plan

No content migration is required because the staged overlay catalog already carries the necessary `base`, `decor`, and `transition` metadata. The change is an in-place runtime behavior update guarded by existing build and test flows. If the widened occupancy rules prove visually incorrect, rollback is limited to reverting the shared road-occupancy helper and road-overlay metadata selection changes.

## Open Questions

- Should three-tile widening be limited to the road intersection vicinity, or should deterministic widening windows also appear farther from spawn along the arms?
- Do we want to preserve the current plus-shaped network exactly, or is slight tapering near the ends desirable as long as deterministic end caps remain legible?
