## Context

The runtime currently loads terrain assets through hard-coded paths in `Game.cpp` and parses terrain classification data through a flat, regex-based loader in `TerrainAutotileSupport.cpp`. That implementation was built around a legacy JSON shape stored directly under `assets/`, but the asset toolchain now emits wrapped classification documents under `assets/output/classifications/` and includes nested per-tile payloads plus tileset metadata such as image path and grid size.

This creates two immediate problems. First, terrain metadata loading is broken because tests and runtime wiring still reference the old source path and schema. Second, the current loader design does not scale to additional classifications such as the new vegetation tileset because every new file would require bespoke parsing and path resolution in runtime code.

The project also has an explicit architectural boundary: gameplay-facing modules remain asset-agnostic, while the SFML runtime shell owns textures, asset loading, and rendering-specific atlas lookups. The design needs to restore compatibility with the generated asset output without pushing asset details into `World` or `OverworldRuntime`.

## Goals / Non-Goals

**Goals:**
- Restore runtime and test compatibility with generated classification files under `assets/output/classifications/`.
- Replace legacy flat parsing with a wrapped-schema document loader that understands tileset metadata and nested tile payloads.
- Introduce a reusable runtime-side tileset asset loading boundary that can support terrain now and vegetation or later classifications without another redesign.
- Keep gameplay modules asset-agnostic and preserve current terrain rendering behavior once metadata is loaded.

**Non-Goals:**
- Implement vegetation sprite rendering or object placement behavior in this change.
- Add automatic directory discovery for arbitrary asset packs beyond the known staged classifications.
- Rework gameplay-facing render snapshots to expose atlas or texture identifiers.

## Decisions

### Introduce a generic runtime tileset asset document and loader

The runtime will gain a small generic layer that loads a wrapped classification document, exposes `tileset` metadata, and indexes tile entries by kind. Terrain-specific code will then build `TerrainTilesetMetadata` from that generic document instead of parsing raw JSON directly.

This is preferred over extending the existing regex parser because the new schema is intentionally structured and nested. A document model gives one place to validate schema-version, tile atlas coordinates, and image/grid metadata, while leaving terrain- and vegetation-specific interpretation to separate adapters.

**Alternatives considered**
- **Patch the terrain regex parser for the new schema:** rejected because it keeps schema handling brittle and terrain-specific, and would not help vegetation or later classifications.
- **Build a full runtime asset registry with directory scanning:** rejected for now because the immediate need is a stable loading boundary for known files, not dynamic asset discovery.

### Resolve staged asset paths from an asset-root boundary

The loader will resolve classification files and image sources from a known runtime asset root instead of assuming every referenced file is colocated with the classification JSON. This matches the generated schema, where `tileset.source.image` is authored asset-root-relative while classifications live under `output/classifications/`.

This avoids baking path rewriting into individual adapters and keeps runtime staging rules explicit.

**Alternatives considered**
- **Resolve paths relative to the classification file location:** rejected because it breaks the current generated layout and couples runtime loading to output folder structure details.
- **Duplicate image paths into runtime code constants:** rejected because it recreates the hard-coded coupling this change is trying to remove.

### Keep terrain rendering as the first consumer and treat vegetation as a loadable next consumer

The first implementation will migrate terrain rendering to the shared loader and ensure the vegetation classification can be loaded by the same boundary later. The runtime does not need to start rendering vegetation sprites in the same change to gain value from the shared loader.

This keeps the change narrow enough to ship while still making the design future-proof for additional classifications.

**Alternatives considered**
- **Render vegetation immediately:** rejected because it expands scope into object rendering, sprite placement, ordering, and collision semantics that are not required to fix the current breakage.

## Risks / Trade-offs

- **Schema drift between asset tooling and runtime parsing** → Centralize wrapped-schema parsing in one runtime document loader and add focused tests against generated classifications.
- **Over-generalizing too early** → Limit the loader to known staged classifications and typed adapters rather than building a dynamic registry or plugin system.
- **Path resolution mistakes between source assets and staged runtime assets** → Define one asset-root resolution rule and update both build staging checks and asset-dependent tests to use it consistently.
- **Hard-coded tile size assumptions surviving the migration** → Prefer tileset grid metadata from the document where practical and only retain constants where they are true rendering invariants.

## Migration Plan

1. Update runtime and test path wiring to reference generated classifications under `assets/output/classifications/`.
2. Add the generic tileset asset document/loader and move terrain metadata construction onto it.
3. Update terrain-specific tests to validate the wrapped schema through the new loader path.
4. Stage any newly required generated classification files for runtime use and keep existing terrain rendering behavior intact.
5. Leave vegetation rendering disabled but accessible through the shared loader for follow-on work.

Rollback is straightforward: revert the loader integration and restore the old hard-coded terrain classification path and parser if needed.

## Open Questions

None. This design resolves the remaining scope choices as follows:

- The first implementation stages both terrain and vegetation classifications so the runtime asset layout is symmetric for known generated classifications, even though only terrain rendering changes in this change.
- `Game.cpp` continues owning direct SFML texture loading in the first implementation, while the shared loader is limited to classification parsing, tileset metadata, and asset-root path resolution.
