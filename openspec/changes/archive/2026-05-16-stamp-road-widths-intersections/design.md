## Context

The current road pipeline in `rpg-sfml` combines three concerns inside a mostly binary occupancy model: road topology generation, tile-space footprint projection, and final visual autotile selection. `RoadNetworkSupport.hpp` defines nodes and segments and also answers whether a tile is road-covered, while `World` and `WorldContent` reuse that same occupancy decision for visible overlays and content exclusion. `GameRenderBatchSupport.cpp` then derives the final atlas role from the visible overlay neighborhood.

That arrangement works for simple two-tile corridors, but it makes width, shoulder, curve, and intersection behavior emerge from hard-coded footprint overlap instead of explicit structural rules. The change introduces a dedicated stamping layer that keeps topology responsible for where roads go, moves tile-shape construction into a world-owned structural pass, and leaves autotile rendering as the last visual-only stage.

Constraints:

- Keep the current module boundaries intact: `World` owns deterministic content and the renderer stays in `GameRenderBatchSupport`.
- Preserve deterministic behavior across repeated launches and repeated frame queries.
- Avoid coupling gameplay-facing systems to atlas coordinates or other renderer-specific data.

## Goals / Non-Goals

**Goals:**

- Represent road intent in topology data without forcing topology types to be the final rasterized road shape.
- Introduce a deterministic road-stamping stage that converts topology plus structural metadata into tile-space road structure.
- Support width rules, shoulders, curves, and intersections as explicit stamped behavior rather than accidental overlap.
- Keep a single stamped road source of truth for visible overlays, content exclusion, and other world queries.
- Preserve autotile rendering as the final pass that chooses atlas cells from stamped local structure.

**Non-Goals:**

- Adding new art assets, new road materials, or graph-specific atlas roles.
- Reworking unrelated terrain autotiling or chunk streaming behavior.
- Introducing author-edited road maps; topology remains deterministic and generated from world-owned data.

## Decisions

### 1. Add a road-stamping layer between topology and rendering

The pipeline will become:

```text
RoadNetwork (intent)
    -> RoadStampField (tile-space structure)
    -> Road overlay rendering (autotile visuals)
```

The stamped field is a world-owned data product, not a renderer-owned convenience cache. This keeps world systems aligned on one structural answer for road coverage.

**Alternatives considered**

- Continue projecting topology directly to boolean occupancy: simpler, but widths and intersections remain implicit and brittle.
- Move stamping into the renderer: would couple gameplay-facing road shape decisions to visual code and break content/query reuse.

### 2. Extend topology with structural metadata, not atlas-facing metadata

`RoadSegment` and relevant node/junction types will gain metadata that describes corridor intent, such as carriage width, shoulder policy, curve treatment, cap style, or intersection priority. The metadata will stay structural and deterministic; it will not include atlas coordinates or render-only role names.

This lets topology express "main road versus narrow spur" and "wide junction versus narrow corridor" while keeping the renderer independent of topology details beyond stamped results.

**Alternatives considered**

- Encode width and intersection behavior only in the stamp pass: loses determinism and intent at the topology boundary.
- Store final visual roles directly on topology elements: mixes gameplay structure with rasterization and art concerns.

### 3. Stamp to structured per-tile road data instead of a single boolean

The stamp output should capture more than `isRoad`. A stamped tile record should describe occupied structure such as carriageway, shoulder, connection directions, curve/intersection participation, and any stable width class needed by downstream systems.

The renderer can still derive local autotile roles from neighboring stamped structure, but it no longer has to reconstruct road intent from sparse occupancy alone.

**Alternatives considered**

- Publish only occupied/not occupied: insufficient for width and intersection readability.
- Precompute final atlas cells in the stamp layer: too render-specific and would make terrain-aware visual selection harder to evolve.

### 4. Keep autotile selection as the final visual pass

`GameRenderBatchSupport.cpp` should remain responsible for selecting atlas cells from staged metadata, underlying terrain surface, and local road structure. The renderer will consume stamped road structure rather than direct topology projection, preserving the existing separation between world data and visual assets.

**Alternatives considered**

- Merge stamping and autotile selection into one step: faster to wire initially, but it collapses structure and appearance back together.
- Publish pre-expanded transition roles from `World`: leaks rendering concepts into world-owned state.

### 5. Make the stamp field the shared query source for world systems

Visible road overlay publication and road-aware content exclusion should both read the stamped road result. This preserves the existing alignment requirement while moving the shared source of truth from binary footprint projection to stamped structure.

**Alternatives considered**

- Let content exclusion keep using topology while rendering uses stamps: would create drift between visible roads and blocked content anchors.

## Risks / Trade-offs

- **[Risk] Wider roads increase ambiguity at bends and junctions** → Mitigation: define deterministic node/segment precedence rules and cover representative T, cross, loop, and endpoint shapes in tests.
- **[Risk] A richer stamp record could sprawl into renderer-specific fields** → Mitigation: keep stamp data structural and forbid atlas-role or texture data in world-owned records.
- **[Risk] Chunk queries may repeatedly restamp the same area** → Mitigation: scope the stamped output to chunk or query regions and reuse deterministic world-owned helpers where practical.
- **[Trade-off] The design adds one more explicit phase** → This increases implementation surface area, but it buys much clearer ownership boundaries and future extensibility for road classes and geometry rules.

## Migration Plan

1. Extend deterministic road topology types with structural metadata required by stamping.
2. Introduce a world-owned road-stamping helper that projects topology into tile-space structural records.
3. Update visible road overlay publication and road-aware content exclusion to consume stamped results.
4. Update render-time road overlay selection to derive visual roles from stamped local structure.
5. Remove or narrow legacy direct-footprint helpers once all callers use the stamped path.

No content migration or save-data migration is expected because the world remains procedurally generated from deterministic inputs.

## Open Questions

- Should shoulders be published as first-class visible road overlay tiles, or should they only influence local transition selection around the carriageway?
- Should stamping operate per queried region, per retained chunk, or behind a small cache keyed by bounds and seed?
- How much structural detail should be exposed publicly versus kept inside `detail` helpers as implementation-private data?
