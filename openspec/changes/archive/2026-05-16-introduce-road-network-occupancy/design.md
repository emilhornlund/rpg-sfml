## Context

Road overlay occupancy is currently derived from spawn-relative band math in `RoadOverlayWorldSupport.hpp`, then shared by `World` and `WorldContent` so visibility and vegetation exclusion stay aligned. That helper was sufficient for a widened plus-shaped spawn road, but it hard-codes the topology itself, which blocks branches, loops, and destination-oriented roads even though the renderer already knows how to compose edges and corners from local occupancy.

This change crosses world generation, road publication, world-content exclusion, and tests. The existing render-side road batch should remain occupancy-driven and surface-aware; the main shift is replacing shape-specific math with a world-owned road network that becomes the authoritative source for occupancy queries.

## Goals / Non-Goals

**Goals:**
- Introduce a deterministic, world-owned road-network model that can express junctions, branches, loops, endpoints, and optional destination anchors.
- Route all road occupancy decisions through that network so `World::hasRoadOverlay()`, visible road overlay publication, and road-aware content exclusion stay in sync.
- Preserve the current render-side autotile approach, where road presentation is derived from local neighboring occupancy and underlying surface type.
- Keep the first network small, deterministic, and easy to reason about within the existing `World` boundary.

**Non-Goals:**
- Introducing pathfinding, NPC routing, or a general gameplay graph API in this slice.
- Replacing the current road overlay tileset or the batched overlay renderer.
- Adding a general-purpose authored map format or external road editor.
- Reworking terrain generation, chunk storage, or player traversability rules around roads.

## Decisions

### Decision: Model roads as a deterministic topology owned by `World`

The runtime will build a small deterministic road network from world seed and spawn context, with explicit nodes and segments rather than deriving occupancy directly from coordinate bands.

- **Why:** This makes topology a first-class world concern and lets the same source describe branches, loops, endpoints, and destinations without encoding those shapes into bespoke occupancy math.
- **Alternative considered:** Keep extending the spawn-centered band helper with more special cases for branches or loops. Rejected because each new motif would further entangle topology decisions with tile-occupancy math and make the source of truth harder to reason about.

### Decision: Keep the network compact and tile-aligned in the first slice

Segments will be expressed in tile-space using short polylines or axis-aligned runs between nodes, with junction footprints and optional destination-center footprints derived from the same topology.

- **Why:** Tile alignment matches the current overlay renderer, makes occupancy queries straightforward, and avoids introducing geometric interpolation that the existing tile atlas cannot represent.
- **Alternative considered:** Use arbitrary world-space splines or diagonal road geometry. Rejected because the current road tileset and autotile logic are grid-based.

### Decision: Project graph topology into occupancy queries instead of storing a second road tile layer

`hasRoadOverlayAt()` and related visibility/content queries will answer road membership by evaluating whether a tile lies inside any segment, junction, endpoint, or destination footprint implied by the network.

- **Why:** This preserves the current world-facing boolean road query contract while moving the source of truth up to topology. It also avoids duplicating the network into a separate authored or chunk-retained tile mask.
- **Alternative considered:** Precompute and retain explicit road-covered tiles per chunk as the source of truth. Rejected for the first slice because the network is small, deterministic, and cheap enough to project on demand.

### Decision: Keep one shared road occupancy helper consumed by `World` and `WorldContent`

The current duplication guard is valuable: visible roads and vegetation suppression must keep calling the same helper boundary even after the helper switches from spawn-relative band logic to road-network evaluation.

- **Why:** Divergence between visible overlays and content exclusion would immediately create visual artifacts such as shrubs on roads or invisible occupancy holes.
- **Alternative considered:** Let `World` own road publication while `WorldContent` reimplements exclusion from graph primitives directly. Rejected because it would duplicate topology-to-occupancy logic across modules.

### Decision: Leave render batching neighbor-driven and topology-agnostic

The renderer will continue to consume published visible road tiles and resolve atlas cells from neighboring occupancy plus surface type, without learning about nodes, segments, or destinations.

- **Why:** The current render path already composes useful transition motifs from local occupancy. A richer occupancy set should improve results without widening the render boundary.
- **Alternative considered:** Teach the renderer explicit graph roles such as endpoint, branch, or district approach. Rejected because topology-specific rendering is unnecessary while local occupancy already encodes the visible shape language.

### Decision: Seed the first network from spawn plus nearby deterministic destinations

The first generator should create a small family of deterministic layouts anchored at spawn, then connect a few nearby destination nodes with optional branch and loop links.

- **Why:** This gives immediate gameplay-readable structure while keeping generation predictable and bounded.
- **Alternative considered:** Generate a fully free-form world-scale road network immediately. Rejected because it adds design surface and tuning complexity before the topology contract is proven.

### Decision: Use synthetic destination targets in the first slice

The first implementation will use internal synthetic destination nodes as road targets rather than trying to align roads with future gameplay landmarks that do not yet exist.

- **Why:** This keeps the topology self-contained and implementable now, while still leaving a clean seam for future landmark systems to replace or drive destination selection later.
- **Alternative considered:** Reserve destination nodes for future authored landmarks only. Rejected because it would delay useful network structure until a separate gameplay system exists.

### Decision: Allow loops only in some deterministic layouts

The generator does not need to guarantee a loop for every seed, but it must guarantee that the same seed always produces the same layout, including whether a loop is present.

- **Why:** This preserves determinism while allowing layout variety across seeds instead of forcing every network into the same topological shape.
- **Alternative considered:** Guarantee at least one loop in every generated network. Rejected because it reduces layout variety and may create awkward small-world shapes for seeds that read better as branched or hub-and-spoke networks.

### Decision: Defer per-chunk occupancy caching until profiling justifies it

The first implementation will evaluate topology-backed occupancy directly from the small generated network instead of adding a per-chunk cached occupancy mask up front.

- **Why:** The network is intentionally compact, and avoiding an extra cache keeps the first slice simpler and easier to validate. A cache can be introduced later if visible-tile scans or vegetation exclusion prove measurably expensive.
- **Alternative considered:** Add a per-chunk occupancy cache in the initial implementation. Rejected because it increases moving parts and invalidation concerns before there is evidence that direct evaluation is too slow.

## Risks / Trade-offs

- **[Risk] A topology-backed network still produces occupancy patterns the current autotile role resolver cannot represent cleanly** → Mitigation: keep the first network tile-aligned and cover junctions, endpoints, loops, and branch approaches in road rendering tests.
- **[Risk] On-demand occupancy projection becomes a hot path when scanning visible tiles and vegetation anchors** → Mitigation: keep the network small in the first slice and allow a later per-chunk occupancy cache if profiling justifies it.
- **[Risk] Endpoint clipping rules from the current helper suppress valid graph-authored terminals** → Mitigation: review and adjust the published-road filtering rules against explicit endpoint scenarios rather than preserving plus-shape assumptions.
- **[Risk] Destination nodes become an implied gameplay API before they are actually used** → Mitigation: treat destination centers as internal world-generation data and only publish occupancy in this change.

## Migration Plan

1. Add an internal road-network data model and deterministic generator within the world/runtime boundary.
2. Replace spawn-centered road band occupancy with topology-backed occupancy evaluation in the shared road helper.
3. Update visible road publication and world-content exclusion to consume the new helper without changing their public contracts.
4. Refresh road/world tests to assert deterministic branches, loops, endpoints, and destination-connected occupancy.
5. Keep rollback simple by reverting the network generator and shared helper back to the current widened-cross implementation if needed.

## Open Questions

- None for the initial implementation slice.
