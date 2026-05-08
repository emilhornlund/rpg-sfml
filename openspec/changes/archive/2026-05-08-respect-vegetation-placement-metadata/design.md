## Context

Vegetation placement currently combines deterministic sampling with hardcoded prototype pools for forest trees, grass trees, forest shrubs, and grass shrubs. The staged vegetation classification already contains richer placement metadata on anchor tiles, including `placeOn` tile constraints and per-biome weights, but the runtime loader does not retain those fields and the world content generator never consults them.

This change crosses the tileset loading boundary, vegetation metadata assembly, and chunk content generation. It also changes runtime content outcomes by allowing metadata-qualified water vegetation such as water lilies and marsh reeds to appear without introducing new gameplay interaction systems.

## Goals / Non-Goals

**Goals:**
- Make staged vegetation classification metadata the source of truth for vegetation prototype eligibility.
- Preserve `placeOn` and `biomes` metadata through loading and vegetation metadata assembly.
- Use the loaded metadata during deterministic chunk content generation so valid water vegetation can be placed.
- Preserve deterministic content identities, geometry handling, and world-facing content queries.

**Non-Goals:**
- Adding shoreline-specific placement rules or new classification fields for shoreline adjacency.
- Redesigning world terrain generation, chunk retention, or render ordering.
- Introducing interaction behavior, simulation state, or runtime randomness into vegetation content.

## Decisions

### Preserve placement metadata in the vegetation asset model

The tileset loading and vegetation metadata layers will retain anchor placement metadata needed at runtime, specifically allowed anchor tile classes and biome weights. This keeps the staged classification authoritative and avoids duplicating placement rules in code.

**Alternatives considered:**
- Keep hardcoded placement pools and only special-case water vegetation. Rejected because it leaves the asset metadata partially ignored and makes future vegetation tuning depend on code edits.
- Read placement metadata directly from the raw asset document during placement. Rejected because placement logic already depends on assembled prototype metadata, and bypassing that layer would duplicate lookup logic.

### Drive prototype eligibility from metadata rather than fixed forest/grass lists

Chunk content generation will build candidate prototype pools from loaded vegetation metadata and filter them by the anchor tile type using `placeOn`. Per-biome weights from `biomes` will control whether a prototype participates for a given anchor tile type.

**Alternatives considered:**
- Retain separate hardcoded tree and shrub arrays and only append metadata-qualified water prototypes. Rejected because it creates two sources of truth and still would not generalize to future placement classes.
- Interpret biome weights only as prototype selection weights after a generic placement pass. Rejected because zero or absent support needs to suppress placement eligibility, not just alter relative selection among already-eligible prototypes.

### Keep chunk-scoped deterministic sampling, but widen supported anchor tiles

The existing deterministic anchor sampling approach remains in place so content identity and stability do not change fundamentally. The implementation should expand supported anchor tile types beyond forest and grass where metadata allows, including water anchors, while still avoiding placement in chunks that truly have no eligible vegetation candidates.

**Alternatives considered:**
- Replace the current placement algorithm with a fully new metadata-driven tiling pass. Rejected because it is higher risk and unnecessary for the requested behavior.
- Continue skipping chunks based on traversability. Rejected because it prevents fully-water chunks from ever producing valid water vegetation.

## Risks / Trade-offs

- **Metadata gaps or malformed values** → Validate and normalize placement metadata as it is loaded so runtime placement fails clearly rather than silently ignoring bad data.
- **Behavior shifts in existing forest and grass placement** → Preserve deterministic sampling structure and use tests to confirm forests remain denser than grass while allowing metadata to refine prototype eligibility.
- **More candidate evaluation per tile** → Precompute grouped prototype pools by anchor tile type so chunk generation does not repeatedly scan every prototype for every anchor tile.
- **Ambiguity around missing biome weights** → Define a consistent fallback, such as treating missing weights as unsupported for that tile type, and capture that explicitly in specs and tests.

## Migration Plan

No data migration or external rollout steps are required. The change applies to the existing staged vegetation classification and should remain backward-compatible with current assets as long as required placement metadata is present where placement is expected.

## Open Questions

No blocking open questions remain for this proposal. Shoreline-specific filtering is intentionally deferred to a separate change if water-edge bias is needed later.
