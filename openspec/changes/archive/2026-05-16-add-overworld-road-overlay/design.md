## Context

The current overworld pipeline renders one terrain layer from `visibleTiles`, then y-sorted generated content and markers. Terrain autotiling already resolves edge roles from neighboring tiles, and the new ground overlay catalog uses a very similar role vocabulary for dirt-road edge blending on top of grass, forest, and sand. At the same time, the existing world and render snapshot boundaries do not expose any secondary tile layer, and the shared tileset loader does not yet preserve `overlay`-classified catalog entries.

This makes road overlays a cross-cutting but still tractable change: the feature touches world data, render snapshots, asset loading, and deterministic content placement, but it does not require the structural traversal, height, or occlusion rules that cliff-style occluders would need.

## Goals / Non-Goals

**Goals:**
- Introduce dirt roads as a deterministic secondary overworld tile layer.
- Preserve the current gameplay/render boundary by keeping world logic texture-agnostic and SFML ownership in render-side helpers.
- Reuse the existing autotile-style neighbor resolution pattern for road edge selection.
- Ensure visible roads read clearly by suppressing conflicting ground vegetation on road-covered tiles.

**Non-Goals:**
- Building a generic multi-layer map editor or arbitrary authored overlay system.
- Introducing cliff, wall, or elevation semantics.
- Changing player traversability because of roads in the first slice.
- Reworking the existing vegetation/player occlusion composite into a general structure occlusion system.

## Decisions

### Roads are world-owned data and snapshot-published render inputs

Road presence will be owned by `World` and exposed through `OverworldRuntime` in the frame snapshot, rather than derived ad hoc in `Game.cpp`.

- **Why:** This matches the existing boundary where gameplay-facing modules publish repo-native render data and the shell only draws from snapshot state.
- **Alternative considered:** Compute roads entirely in render code from visible terrain. Rejected because vegetation suppression and future road-aware gameplay would then need duplicate road logic outside `World`.

### The first slice uses a narrow road-overlay model, not a generic overlay framework

The first implementation will add a dedicated road overlay representation oriented around the staged `dirt_road` overlay catalog instead of introducing a fully generic overlay taxonomy.

- **Why:** The catalog and current scope are both narrow, and a focused model reduces schema and runtime complexity while still leaving room to generalize later if more overlays appear.
- **Alternative considered:** Add a generic N-layer overlay framework immediately. Rejected because it expands API and data-model surface without a second concrete use case.

### Overlay metadata gets a dedicated runtime metadata helper

Overlay catalog entries will be parsed by the shared tileset loader, but runtime atlas selection will use a dedicated road-overlay metadata helper rather than reusing `TerrainTilesetMetadata` directly.

- **Why:** The road catalog overlaps with terrain autotile concepts but has a different content model: base road tiles plus surface-specific transition edges, without water animation or the full terrain transition matrix.
- **Alternative considered:** Force the road catalog through `TerrainTilesetMetadata`. Rejected because its validation and invariants are terrain-specific and would make the overlay catalog look like malformed terrain data.

### Rendering adds one extra batched pass between terrain and sorted content

The game shell will build a separate road-overlay vertex array and draw it after terrain but before vegetation and player markers.

- **Why:** Roads are tile-aligned ground detail, so they belong visually above terrain and below taller content. A batched pass keeps the rendering model aligned with the current geometry-based terrain path.
- **Alternative considered:** Fold road atlas selection into terrain batching. Rejected because the road tiles come from a different texture and should remain an optional secondary layer.

### Road edge selection uses road occupancy plus underlying surface type

Each visible road tile will resolve its atlas cell from:
1. whether the tile is marked as road,
2. the underlying terrain `TileType`,
3. neighboring road occupancy,
4. deterministic base variation when no transition edge role applies.

- **Why:** This directly matches the road catalog shape and keeps edge blending driven by the road mask rather than by terrain transitions.
- **Alternative considered:** Infer road edges from terrain categories alone. Rejected because terrain does not encode road membership.

### Ground vegetation suppression is part of the initial slice

Deterministic ground vegetation generation will treat road-covered tiles as ineligible anchors for dense ground placements in the first implementation.

- **Why:** Without this, shrubs and similar ground clutter can spawn onto visible roads, making the feature look immediately wrong.
- **Alternative considered:** Ship rendering first and defer content cleanup. Rejected because the visual conflict is a core readability issue, not a polish-only follow-up.

## Risks / Trade-offs

- **[Road source shape is underspecified]** → Start with a deliberately simple deterministic generator and keep the road data model separate from rendering so a later authored or network-based source can replace it.
- **[Too much abstraction too early]** → Keep the first slice focused on dirt-road overlays rather than solving generic overlay layering.
- **[Snapshot growth adds render-side wiring]** → Publish only road entries needed for drawing and vegetation suppression, not extra authoring metadata.
- **[Catalog parsing changes could blur asset models]** → Parse `overlay` tiles through the shared loader but hand off selection semantics to a dedicated overlay metadata helper.

## Migration Plan

1. Extend tileset asset loading to preserve overlay-classified entries and load the ground overlay atlas.
2. Add world-owned road data and publish visible road overlay entries through `OverworldRenderSnapshot`.
3. Add overlay metadata resolution and a dedicated batched render pass.
4. Make vegetation placement road-aware so ground cover does not occupy road tiles.
5. Keep the change internal to the executable; no external save or content migration is required.

Rollback is straightforward: remove the overlay resource loading and snapshot/render wiring, returning the overworld to terrain-plus-content rendering.

## Open Questions

- Should the first road generator focus on a small spawn-adjacent proof of concept or a chunk-wide deterministic pattern visible across broader exploration?
- Should sparse props and trees also be prevented from anchoring to roads in the first slice, or is suppressing dense ground cover sufficient for the initial change?
