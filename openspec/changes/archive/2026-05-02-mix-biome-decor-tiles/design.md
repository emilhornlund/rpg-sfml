## Context

The current terrain renderer resolves each visible tile into either an autotile transition cell or a base terrain variant loaded from the overworld terrain tileset classification metadata. This keeps rendering deterministic and gameplay-facing systems texture-agnostic, but it also leaves large interior biome regions visually repetitive.

The tileset classification metadata already includes biome-specific `decor` entries for grass, forest, and sand. Those entries should participate in the same fallback path that currently chooses only `base` variants. The change must preserve the same procedural guarantees as terrain rendering today: stable selection for the same world configuration, world coordinates, and neighborhood regardless of frame order, camera motion, or chunk retention order.

## Goals / Non-Goals

**Goals:**
- Extend metadata-backed terrain rendering so non-transition biome tiles can resolve to either a base variant or a decor variant.
- Keep selection deterministic from stable procedural inputs, including the world seed.
- Keep autotile transitions authoritative so shoreline and biome-edge rendering remains unchanged.
- Preserve existing gameplay-facing terrain semantics, including `TileType`, traversability, chunk metadata, and render snapshot structure.

**Non-Goals:**
- Introducing decor as a separate gameplay entity, overlay layer, or retained world-data channel.
- Changing biome generation, chunk generation rules, or traversability rules.
- Reworking asset formats beyond consuming existing `decor` classification entries.
- Matching decor frequency to the raw asset count ratio in the metadata.

## Decisions

### Decision: Treat decor as part of the default terrain appearance pool
- **Choice:** Non-transition grass, sand, and forest tiles will resolve through a shared default appearance selector that chooses either a base cell or a decor cell for that biome.
- **Why:** This matches the desired behavior exactly: decor replaces the normal base appearance when selected, but the underlying terrain tile still behaves the same.
- **Alternative considered:** Add decor as a second render pass or separate prop layer. Rejected because it introduces new presentation semantics and implies future gameplay ownership that is out of scope for this change.

### Decision: Keep decor selection in the render-side metadata path
- **Choice:** The renderer and terrain-tileset helper code will own decor selection, alongside the existing base-variant and autotile selection logic.
- **Why:** The current architecture intentionally keeps gameplay modules texture-agnostic. Extending render-side metadata selection preserves that boundary and avoids adding prop identity to `World` or `OverworldRuntime`.
- **Alternative considered:** Generate decor identities in chunk data and retain them in `World`. Rejected because the requested behavior does not require gameplay-aware prop state.

### Decision: Use deterministic weighted selection rather than uniform pooling
- **Choice:** The fallback path will use a deterministic decor roll with a low decor share, then choose within the selected pool for the biome.
- **Why:** The metadata contains many more decor entries than base entries for some biomes, especially grass. Uniform selection across a merged pool would make decor appear too often and would overwhelm the terrain.
- **Alternative considered:** Merge base and decor arrays and select uniformly. Rejected because the visual density would be driven by asset-count imbalance instead of an intentional ratio.

### Decision: Include world seed in default appearance selection
- **Choice:** The default appearance selector will derive its deterministic choice from world seed, tile coordinates, tile type, and pool-selection context.
- **Why:** Terrain generation is already seed-driven. Including the seed keeps decor selection aligned with the same procedural contract and allows different seeds to produce different decor layouts for identical coordinates.
- **Alternative considered:** Reuse the current coordinate-only base variant hash unchanged. Rejected because it does not fully match the seed-aware procedural generation behavior expected for this change.

### Decision: Restrict decor to non-transition tiles
- **Choice:** Decor selection will only apply when no autotile transition cell is selected for the tile.
- **Why:** Transition rendering already encodes biome boundaries and shoreline shapes. Allowing decor on those tiles would blur edges and compete with the autotile system.
- **Alternative considered:** Allow decor on all walkable tiles. Rejected because it would create noisy coastlines and mixed biome borders.

## Risks / Trade-offs

- **[Visual density drifts too high in some biomes]** → Use an explicit decor-weight rule instead of metadata-count-driven mixing, and cover the chosen behavior with render-selection tests.
- **[Seed-aware selection changes the exact base variant chosen for existing worlds]** → Scope the deterministic selector update carefully and document that the change intentionally updates terrain presentation while preserving gameplay semantics.
- **[Metadata gaps for some biome/pool combinations]** → Keep validation strict for required base variants and treat decor as optional per biome so rendering still has a valid fallback.
- **[Render helper complexity grows]** → Keep the new logic inside the existing terrain metadata and atlas-cell selection helpers rather than spreading pool-selection rules through the game loop.

## Migration Plan

1. Extend terrain tileset metadata loading to retain decor entries by biome alongside base variants.
2. Replace the non-transition base-only fallback with deterministic weighted base-or-decor selection.
3. Update terrain rendering tests to cover metadata loading, deterministic selection, and transition precedence.
4. Ship without gameplay migrations because terrain ownership and runtime interfaces remain unchanged.

## Open Questions

- The initial decor share should remain conservative; current exploration settled on a low-frequency mix rather than biome-specific tuning, but the exact constant can still be adjusted during implementation if visual review shows one biome is too sparse or too busy.
