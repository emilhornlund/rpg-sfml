## Context

The current overworld content path generates a small number of deterministic instances per chunk from coarse chunk metadata and renders them as a single generated-content layer between terrain and player presentation. That model works for sparse points of interest, but it does not support biome-driven forests, scattered grassland vegetation, or large multi-tile vegetation objects whose visual footprint extends beyond a single tile.

The project now has an overworld vegetation tileset classification that already describes object families, anchor tiles, and per-part offsets for multi-tile objects. The design needs to preserve the existing separation of concerns: terrain remains terrain, world-owned content remains deterministic and data-only, and the outer shell continues to render from gameplay-owned snapshot data without reaching back into `World`.

## Goals / Non-Goals

**Goals:**
- Add deterministic vegetation placement that is stable for a given world seed and world-space coordinates.
- Produce denser, more clustered vegetation in forest biomes and sparse, isolated vegetation in grass biomes.
- Use the staged vegetation tileset classification as the authoritative source for vegetation atlas metadata.
- Preserve chunk retention and world-facing query boundaries while ensuring chunk boundaries do not change vegetation outcomes.
- Support y-sorted object rendering so vegetation and player presentation can overlap naturally.

**Non-Goals:**
- Adding vegetation interaction logic, harvesting, collision bodies finer than tile traversability, or scripted behaviors.
- Replacing the terrain autotile pipeline or moving terrain rendering into the vegetation system.
- Introducing runtime randomness, streaming systems outside the existing retained-chunk model, or a full scene graph abstraction.
- Solving every future object type in this change; the focus is overworld vegetation and the rendering/data structures it needs.

## Decisions

### 1. Vegetation placement is driven by world-space deterministic sampling

Vegetation placement will be decided from stable world-space sampling inputs derived from `(seed, tileX, tileY)` or a coarser placement-cell coordinate, not from runtime randomness and not from chunk summary statistics. Chunks will continue to retain generated vegetation, but retained content will be the cached result of world-space sampling rather than the source of truth for the placement decision.

This avoids visible seams at chunk boundaries and makes repeated queries for the same world coordinates resolve to the same vegetation records regardless of chunk load order.

**Alternatives considered:**
- Continue using chunk metadata to place more instances per chunk: rejected because density would still depend on chunk-owned summaries and would create coarse, artificial forest shapes.
- Use runtime RNG during chunk generation: rejected because generation order would affect results and deterministic replay would be harder to preserve.

### 2. Trees use coarse anchor sampling; small vegetation uses finer deterministic sampling

Large trees will be placed from deterministic anchor candidates sampled on a coarse world-space grid so placement stays stable while still enforcing natural spacing. Smaller vegetation such as bushes and reeds can use denser tile-scale or small-cell sampling with biome-gated thresholds.

This hybrid approach matches the asset set and keeps forests readable: trees get spacing and clustering, while undergrowth can appear denser without collapsing into tile-by-tile noise.

**Alternatives considered:**
- Pure per-tile sampling for all vegetation: rejected because large trees would overpopulate and require many more rejection rules to maintain spacing.
- Pure coarse-grid sampling for all vegetation: rejected because bushes and other small props would look too sparse and rigid.

### 3. Vegetation metadata is built around object prototypes anchored at ground-contact tiles

The vegetation classification already groups multi-tile objects with `object.id`, `family`, `role`, and per-part offsets. Runtime metadata should therefore normalize the classification into vegetation prototypes keyed by object id, with one anchor definition and zero or more part tiles. World content instances will reference a prototype and an anchor tile/world position rather than flattening the object into unrelated per-tile records.

This keeps identity stable, makes large-object bounds easy to compute, and gives rendering a natural sort point at the anchor/base tile.

**Alternatives considered:**
- Flatten every vegetation part into an independent world-content instance: rejected because identity, visibility, and y-sorting would become more complex for multi-tile trees.
- Special-case only trees and ignore the generic classification structure: rejected because the current vegetation metadata already supports a reusable object prototype model.

### 4. Chunk ownership is by vegetation anchor tile, with visibility overscan for large objects

Each vegetation instance will belong to the chunk that owns its anchor tile. During visible-content queries, the world will examine the intersecting chunk range plus a bounded overscan derived from the maximum vegetation footprint extent so large trees anchored just outside the immediate view-chunk set can still be returned when their rendered bounds intersect the camera frame.

This preserves the existing retained-chunk cache structure while handling cross-chunk tree canopies correctly.

**Alternatives considered:**
- Duplicate vegetation into every chunk it overlaps: rejected because it complicates stable identity and cache invalidation.
- Replace chunk retention with a view-driven global sampler: rejected because it would cut against the current world retention model and broaden the change significantly.

### 5. Render snapshots publish sort-ready object entries and the shell performs y-sorted object rendering

Terrain remains a dedicated terrain pass. Vegetation content and the player marker will be rendered as object entries that carry the world-space data needed to compute a stable y-sort key from the anchor/base position. The game shell will draw these object entries in y-sorted order instead of always drawing all generated content before the player.

This keeps gameplay free of SFML types while enabling the standard RPG depth effect of the player moving behind or in front of trees.

**Alternatives considered:**
- Keep a fixed pass order and split trees into special under/over layers: rejected because it does not generalize well and would require more shell-specific rules.
- Move sorting entirely into `Game`: rejected because snapshot data must still publish enough ordering context for the shell to render without re-deriving gameplay meaning.

### 6. Asset staging explicitly includes vegetation runtime files

The staged executable asset layout will include the vegetation tileset image and vegetation classification document alongside the existing terrain tileset assets. Runtime asset support will expose a dedicated vegetation metadata loading path parallel to terrain metadata loading.

This keeps runtime loading consistent with the project’s staged asset model and allows tests to validate vegetation asset availability the same way they validate terrain assets.

## Risks / Trade-offs

- **Large-object visibility expands visible-content queries** → Limit overscan to the maximum prototype extent from vegetation metadata and continue using chunk retention to bound generation work.
- **Dense forests can create too many records per frame** → Use coarse anchor sampling for trees, biome-gated thresholds, and prototype-family filtering to cap candidate counts.
- **Y-sorting can introduce unstable draw order for equal Y values** → Use a deterministic secondary key such as stable instance id after the primary sort key.
- **Biome-specific rules may need tuning after first implementation** → Keep density and clustering rules centralized in the vegetation sampler so values can be tuned without reshaping the architecture.
- **Vegetation metadata may contain future object families beyond this feature** → Normalize around the generic object prototype model but scope placement rules in this change to vegetation-relevant families and tags.
