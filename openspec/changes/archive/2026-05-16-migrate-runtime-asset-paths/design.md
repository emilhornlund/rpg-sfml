## Context

The game code already treats generated catalog JSON files under `assets/output/catalogs/` as the source of truth for terrain and vegetation metadata, but several runtime and build entry points still assume an older flat asset layout. Terrain and vegetation textures are loaded from hardcoded root-level PNG paths, the player spritesheet is staged and resolved from the asset root, and asset-dependent checks still assert the legacy staged file locations.

The asset toolchain has already moved runtime-ready files into `output/` and now emits distinct catalog metadata for authored source assets and runtime atlas assets. This change needs to align the C++ runtime boundary, build staging, and repository checks with that contract without broadening the game architecture.

## Goals / Non-Goals

**Goals:**

- Make runtime terrain and vegetation texture loading follow the tileset catalog's runtime image metadata.
- Stage all shipped runtime assets from the repository-root `assets/output/` tree into the executable-local `assets/output/` tree.
- Move non-catalog runtime asset helpers and checks to the new spritesheet and tileset output directories.
- Preserve the existing gameplay and rendering module boundaries while updating the asset bootstrap path contract.

**Non-Goals:**

- Changing the asset converter schema beyond consuming the already-added runtime image field.
- Reworking metadata parsing for unrelated tile kinds or gameplay systems.
- Introducing a new asset database, manifest format, or generalized loader abstraction.

## Decisions

### Resolve tileset textures from catalog runtime metadata

Terrain and vegetation textures should be loaded from the wrapped catalog document rather than from separate hardcoded filename helpers. The loader already parses catalog metadata and exposes a resolved image path; this change should extend that path resolution to use the explicit runtime image field emitted by the asset pipeline.

This keeps a single source of truth for tileset runtime locations and prevents the bootstrap layer from duplicating filenames that now live in the catalog contract.

Alternatives considered:

- Keep separate tileset filename helpers and just retarget them to `output/tilesets/`: rejected because it duplicates catalog metadata and leaves runtime tileset resolution split across two sources of truth.
- Overwrite authored source metadata with runtime paths everywhere: rejected because the catalog intentionally preserves source traceability separately from runtime output paths.

### Keep path helpers for non-catalog assets, but move them under output directories

The player spritesheet and debug font are not loaded through wrapped tileset catalogs, so their runtime path helpers should remain explicit. The font helper already points at `output/fonts/`; the player spritesheet helper should move to `output/spritesheets/`.

This keeps the change narrow and consistent with the current codebase rather than inventing a broader asset manifest layer.

Alternatives considered:

- Introduce a generic manifest for all runtime assets: rejected because it expands scope beyond the current path migration.
- Leave the spritesheet at the root while only moving tilesets: rejected because the new asset layout explicitly consolidates runtime-ready files under `output/`.

### Stage build outputs to mirror the repository runtime layout exactly

`src/main/CMakeLists.txt` should copy runtime tilesets, runtime catalogs, runtime spritesheets, and fonts from the repository-root `assets/output/` tree into the executable-local `assets/output/` tree. Tests and staging checks should validate that mirrored structure directly.

Mirroring the repository layout into the executable output reduces special cases and makes runtime path resolution consistent between source-backed tests and built artifacts.

Alternatives considered:

- Flatten the staged output back to the asset root during build: rejected because it reintroduces the mismatch this change is meant to remove.
- Stage both old and new paths temporarily: rejected because it prolongs ambiguity and weakens the tests.

## Risks / Trade-offs

- **Catalog/runtime coupling increases** → Mitigation: keep the change limited to terrain and vegetation, where catalogs are already mandatory for metadata loading.
- **Legacy tests may implicitly depend on root-level paths** → Mitigation: update all path and staging assertions together so the executable layout has one validated contract.
- **`loadFromFile` path resolution remains less explicit than `loadFromAssetRoot`** → Mitigation: keep runtime code on the asset-root-based loading path and treat direct file loading as a secondary testing utility.

## Migration Plan

1. Update the delta specs for `runtime-asset-layout` and `tileset-asset-loading` to define the new staged runtime layout and runtime image resolution behavior.
2. Adjust the asset loader and bootstrap support so terrain and vegetation textures resolve from catalog runtime metadata, and move the player spritesheet path helper to `output/spritesheets/`.
3. Update build staging to mirror `assets/output/` into the executable-local asset directory.
4. Update asset-dependent tests and staging checks to validate the new layout and catalog-driven texture resolution.
5. If rollback is needed, restore the previous root-level staging and image resolution behavior together so runtime and tests stay aligned.

## Open Questions

- Should direct `TilesetAssetDocument::loadFromFile(...)` also resolve runtime images from the catalog's parent tree in a stricter way, or is asset-root-based runtime loading sufficient for the current codebase?
