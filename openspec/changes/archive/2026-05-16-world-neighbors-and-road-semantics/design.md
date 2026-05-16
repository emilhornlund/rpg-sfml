## Context

The current overworld rendering pipeline still has an asymmetry between what the world knows and what the renderer knows. `World` can answer terrain and stamped-road queries for any tile coordinate, but `GameRenderBatchSupport` reconstructs terrain neighbor context from only the visible tile subset. When a tile sits on the viewport edge, missing off-screen neighbors are treated as if they matched the current tile, which can suppress or invent transitions. Roads are in a better state because stamped neighbor occupancy is already computed from world queries, but the topology feeding that stamped field still uses coarse generation-oriented enums (`Spawn`, `Junction`, `Destination`, `Main`, `Branch`, `Connector`) that do not cleanly express the richer local structure the stamping layer now supports.

This change crosses the gameplay snapshot boundary, world-owned road topology/stamping helpers, and the final render batch builder. It must preserve deterministic behavior, keep renderer-specific atlas concepts out of world code, and reuse the current tileset so generation becomes more expressive without requiring new art.

## Goals / Non-Goals

**Goals:**

- Eliminate viewport-edge terrain and road autotile artifacts by sourcing render neighbor context from world-backed data instead of the visible subset alone.
- Extend the render snapshot boundary with structural neighbor data so the outer shell remains render-ready without re-querying gameplay modules.
- Separate road segment importance semantics from node topology semantics so generation can express trails, roads, main roads, dead ends, corners, tees, crosses, and plazas explicitly.
- Feed the richer road semantics into the existing stamping layer so world queries, content exclusion, and road rendering stay aligned on one structural source of truth.
- Keep the current atlas and per-tile autotile selection model intact.

**Non-Goals:**

- Adding new road materials, atlas roles, or art assets.
- Replacing the existing snapshot boundary with direct renderer access to `World`.
- Reworking unrelated chunk retention, terrain generation, or player movement behavior.
- Designing a hand-authored map format or abandoning deterministic procedural road generation.

## Decisions

### Decision: Publish world-backed neighbor context through render snapshots

Visible terrain and road overlay entries will carry the neighbor information needed for final atlas selection, rather than forcing the renderer to infer missing off-screen neighbors from the visible subset. Terrain entries will publish neighboring tile types for the eight surrounding positions for every visible tile, and road overlay entries will publish neighboring stamped-road occupancy derived from the world-owned stamped field.

This preserves the render snapshot boundary: `OverworldRuntime` remains the only gameplay-owned publisher, while the renderer stays a pure consumer of render-ready data.

**Alternatives considered**

- Re-query `World` from `GameRenderBatchSupport`: rejected because it breaks the snapshot boundary and couples rendering back to gameplay modules.
- Publish terrain neighbors only for tile types that currently participate in autotile transitions: rejected because it adds conditional snapshot rules, complicates the render contract, and makes future terrain-transition expansion harder.
- Increase visible overscan until edge artifacts disappear: rejected because it treats a data-contract issue as a camera-tuning workaround and still leaves correctness implicit.

### Decision: Split road anchor roles from structural topology semantics

Road topology will distinguish at least two semantic axes:

- **segment class**: `trail`, `road`, `main_road`
- **node topology**: `dead_end`, `corner`, `tee`, `cross`, `plaza`

Spawn and destination will remain explicit anchor intent in a separate field, but they should no longer stand in for the node's structural shape. The structural shape used by stamping must not be conflated with gameplay anchors, while the generator should still be able to reason directly about spawn-anchored and destination-anchored nodes without re-deriving that intent from layout rules.

**Alternatives considered**

- Keep the current enums and infer richer structure only inside stamping: rejected because it hides generator intent and makes richer layouts harder to reason about and test.
- Infer anchor intent from higher-level generation rules only: rejected because spawn/destination meaning is stable, useful, and clearer when published explicitly than when reconstructed later from context.
- Replace all node metadata with a single “junction kind” enum that mixes anchors and shapes: rejected because `spawn`/`destination` and `tee`/`cross` answer different questions.

### Decision: Let stamping consume semantics, but keep stamping structural

The road-stamping layer will translate the richer segment and node semantics into shared tile-space structure such as corridor width, widening, junction treatment, end-cap treatment, and plaza footprints. Plazas will be represented as a dedicated node topology, while generic widening remains a separate structural policy that can be applied to segments or nodes without implicitly turning them into plazas. The stamped result remains structural data, not render-role data, so downstream systems can derive occupancy and local connectivity without leaking atlas concepts into world code.

This keeps the current architectural split intact:

```text
Topology semantics
    -> stamped structural field
    -> snapshot neighbor context
    -> renderer chooses atlas cells
```

**Alternatives considered**

- Store final atlas roles in the stamped field: rejected because it hard-codes art decisions into world-owned data.
- Treat every widened anchor or widened junction as a plaza: rejected because it would overload “plaza” into a generic width modifier instead of reserving it for a distinct hub-like topological shape.
- Keep stamping boolean-only and add more render heuristics: rejected because it would reintroduce fragile visual inference instead of improving the shared structural language.

### Decision: Road overlays continue to compose motifs from local structure

Even with richer road semantics, the renderer will keep selecting one tile at a time from the existing surface-aware transition/base/decor metadata. End caps, branches, corners, crosses, and plazas should emerge from richer stamped local structure plus world-backed neighbors, not from adding graph-specific atlas roles.

This lets the generator become more expressive first, while keeping the current asset pipeline stable.

**Alternatives considered**

- Add new atlas roles for each topological motif now: rejected because the current tileset is already intended to compose motifs from local structure.
- Render bespoke multi-tile motifs directly from topology: rejected because it would sidestep the existing batch/autotile system instead of strengthening it.

## Risks / Trade-offs

- **[Risk] Snapshot payloads become more verbose** → Mitigation: publish only compact fixed-size neighbor data needed for rendering and keep it limited to visible entries.
- **[Risk] Richer topology semantics create ambiguous stamping rules at bends or wide hubs** → Mitigation: define deterministic precedence between segment class, node topology, and anchor metadata, and cover representative dead-end/corner/tee/cross/plaza cases in specs and tests.
- **[Risk] Terrain and road neighbor data drift from the world source of truth** → Mitigation: make `World` the single publisher of neighbor context used by `OverworldRuntime`, rather than rebuilding neighbor sets in multiple layers.
- **[Trade-off] More structure is exposed in snapshot types** → This slightly enlarges the render boundary, but it removes renderer-side guesswork and keeps correctness inside gameplay-owned code.

## Migration Plan

1. Extend the world/render snapshot types to carry world-backed terrain neighbor types and stamped road neighbor occupancy.
2. Update `World` and `OverworldRuntime` to publish that data for visible entries.
3. Refine road topology semantics to distinguish segment class from node topology and route those semantics into the stamping helpers.
4. Update render batching to consume published neighbor context instead of reconstructing it from visible subsets.
5. Remove or narrow legacy visible-subset neighbor fallback helpers once snapshot-driven rendering is in place.

Rollback is straightforward because the change is internal to deterministic generation and rendering boundaries: revert the richer snapshot fields and topology semantics together, restoring the current visible-subset inference path.

## Open Questions

- None currently.
