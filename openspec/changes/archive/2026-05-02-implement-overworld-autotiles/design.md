## Context

The current overworld renderer owns a shared terrain texture, but it still hardcodes one atlas cell per `TileType` in `Game.cpp`. That model was sufficient for the first terrain sheet, but it cannot use the updated terrain atlas and JSON classification that now describe multiple base variants, directed transition sets, and animated water-adjacent tiles.

This change needs to preserve the existing gameplay boundary. `World`, `OverworldRuntime`, and render snapshots should continue to expose terrain as repo-native `TileType` values and visible-tile geometry. The new behavior belongs in the render layer: load the updated atlas metadata, inspect neighboring terrain categories, normalize the selected terrain pair, compute an autotile role, and map that to the correct atlas tile or animation frame.

## Goals / Non-Goals

**Goals:**
- Replace the fixed terrain atlas lookup with metadata-driven autotile rendering for the updated overworld tileset.
- Keep terrain ownership and generation unchanged so the feature remains a render-only concern.
- Support deterministic base terrain variation for grass, sand, and forest.
- Support deterministic directed transition selection for the supported terrain pairs.
- Support animated water transitions without exposing animation concerns to gameplay modules.

**Non-Goals:**
- Changing biome generation thresholds, tile categories, chunk retention, or player traversal rules.
- Introducing new gameplay-facing terrain identifiers beyond the existing `TileType` values.
- Adding a generic external tileset or map-loading pipeline.
- Reworking the render snapshot contract to include raw atlas coordinates.

## Decisions

### Decision: Add a render-side terrain atlas metadata helper

The implementation should add a small helper owned by the game shell or runtime support layer that loads the updated terrain tileset classification JSON and builds lookup tables for:
- base terrain variants by terrain category
- autotile transitions by directed terrain pair and autotile role
- animated water transition frames by directed terrain pair and role

This keeps asset parsing and atlas metadata out of gameplay modules while removing hardcoded sprite rectangles from `Game.cpp`.

**Alternatives considered:**
- Hardcode the updated atlas coordinates in C++: rejected because the new sheet already has an explicit classification file and hardcoding would make future art changes brittle.
- Push atlas metadata into `World` or render snapshots: rejected because those modules should remain texture-agnostic.

### Decision: Keep world generation and snapshots on base `TileType`

Neighbor-aware autotile choice should happen during rendering from the existing visible tile data. The world continues to answer `TileType` queries, and the renderer derives presentation-specific roles from neighboring tile categories.

This avoids inflating the world model with render-only states such as atlas coordinates, transition roles, or animation frame ownership.

**Alternatives considered:**
- Precompute render variants during world generation: rejected because variants depend on atlas data and would couple world state to presentation.
- Store resolved render variants in render snapshots: rejected because it pushes tileset-specific concerns across the runtime boundary.

### Decision: Use canonical directed terrain pairs with explicit priority

The renderer should normalize adjacent terrain into a small set of canonical directed pairs that match the updated atlas:
- `grass->sand`
- `forest->sand`
- `grass->forest`
- `grass->water`
- `forest->water`
- `sand->water`

When multiple neighboring terrain categories compete, the renderer should choose the highest-priority transition target using a deterministic priority order:

`water > sand > forest > grass`

This matches the current biome hierarchy, keeps border selection stable, and avoids ambiguous multi-terrain seams.

**Alternatives considered:**
- Require both pair directions in assets: rejected because the atlas and classification already use canonical direction.
- Blend multiple transition targets on a single tile: rejected because the current atlas and render path assume one tile selection per world tile.

### Decision: Derive one of fourteen autotile roles from neighboring terrain

For a selected directed pair, the renderer should inspect neighboring tiles and reduce the local pattern to one of the supported roles:

- `outer_top_left`
- `outer_top_right`
- `outer_bottom_left`
- `outer_bottom_right`
- `single_tile`
- `top_left`
- `top`
- `top_right`
- `left`
- `center`
- `right`
- `bottom_left`
- `bottom`
- `bottom_right`

The role resolver should be deterministic and table-driven so tests can assert specific roles for representative neighbor masks.

**Alternatives considered:**
- Use raw atlas coordinates directly as logic outputs: rejected because roles are easier to reason about and test.
- Infer roles from diagonal neighbors only: rejected because the atlas includes edge and isolated-tile states that require cardinal-neighbor ownership.

### Decision: Keep variation and animation deterministic

Base-terrain variation and water transition frame selection should remain deterministic for a given terrain layout and time source. Base variation can derive from stable tile coordinates and terrain category. Water animation should advance from a shared frame clock, while the chosen role and pair remain stable unless neighboring terrain changes.

**Alternatives considered:**
- Randomize variants at render time: rejected because visible tiles would flicker across frames.
- Keep water static for the first iteration: rejected because the atlas already provides animated water transition frames and the updated tileset is meant to use them.

## Risks / Trade-offs

- **[Risk] Transition semantics in the classification JSON may not match the first implementation's role assumptions** → Mitigation: encode pair normalization and role mapping in focused tests using the classification names and expected coordinates.
- **[Risk] Rendering neighbors outside the immediate visible set may require extra world queries near camera edges** → Mitigation: compute roles from world tile queries or from a bounded overscan so edge tiles still resolve correctly.
- **[Risk] Water lacks standalone base entries in the current classification** → Mitigation: define one canonical open-water lookup strategy, such as treating the `center` role in each `*->water` set as the interior water tile.
- **[Risk] Adding JSON parsing increases startup complexity** → Mitigation: load and validate atlas metadata once during initialization and fail fast if required entries are missing.
- **[Trade-off] A render-time resolver is slightly more complex than fixed atlas mapping** → Mitigation: keep the logic isolated in small helpers instead of growing `Game.cpp`.

## Migration Plan

1. Stage the updated terrain atlas and classification JSON in the existing runtime asset pipeline.
2. Introduce the terrain atlas metadata loader and validate required base and transition entries during startup.
3. Replace the fixed terrain rectangle lookup with render-time autotile resolution while preserving the current render snapshot inputs.
4. Add focused tests for pair normalization, role selection, deterministic variation, and water animation frame lookup.
5. Remove the old fixed-atlas terrain mapping once the new render path is in place.

Rollback is straightforward: revert the renderer to the fixed `TileType -> atlas cell` path and switch assets back to the previous terrain tileset.

## Open Questions

- Should open-water interior always reuse a canonical `*->water` `center` tile, or should the tileset later grow explicit `water_base_*` entries?
- Should deterministic base variation choose among all classified variants for a terrain, even when some terrains currently have more base entries than others?
- Should water animation frame timing be tied to wall-clock frame time in `Game`, or exposed through a small runtime support helper for easier testing?
