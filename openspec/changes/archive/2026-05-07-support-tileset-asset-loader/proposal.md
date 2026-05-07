## Why

The runtime still assumes a single terrain classification JSON with the old flat structure at `assets/`, but generated classifications now live under `assets/output/classifications/` and use a wrapped schema with nested tile payloads. This leaves terrain metadata loading broken today and makes every new classification file require more hard-coded runtime plumbing.

## What Changes

- Update runtime asset loading to read terrain classifications from the generated `assets/output/classifications/` layout instead of the legacy root JSON path.
- Replace the terrain metadata loader's flat regex-based parsing with wrapped-schema parsing that understands `schemaVersion`, `tileset`, `tiles`, and nested `terrain` payloads.
- Introduce a reusable tileset asset document/loader boundary so runtime code can load classifications and associated tileset metadata without hard-coding one-off file assumptions.
- Preserve the current gameplay/render boundary by keeping gameplay modules asset-agnostic while the runtime shell owns classification loading, atlas metadata, and texture resolution.
- Prepare the runtime to accept additional classification files, including the new vegetation tileset classification, without requiring another loader redesign.

## Capabilities

### New Capabilities
- `tileset-asset-loading`: Load wrapped tileset classification documents and resolve staged runtime asset paths through a reusable tileset asset loader that can support multiple classification files.

### Modified Capabilities
- `runtime-asset-layout`: The staged runtime asset layout now includes generated classification files under `assets/output/classifications/`, and asset-dependent checks use that source layout.
- `overworld-tileset-rendering`: Terrain rendering now resolves terrain atlas metadata through the reusable tileset asset loading boundary instead of a single hard-coded classification file.
- `overworld-autotile-rendering`: Autotile metadata loading now consumes the wrapped terrain classification schema rather than the legacy flat entry format.

## Impact

- Affected code: `src/main/Game.cpp`, `src/main/TerrainAutotileSupport.cpp`, `src/main/CMakeLists.txt`, `tests/CMakeLists.txt`, asset-staging checks, and new runtime-side tileset asset loading helpers.
- Affected assets: generated classification files under `assets/output/classifications/`, especially `overworld-terrain-tileset-classification.json` and `overworld-vegetation-tileset-classification.json`.
- No gameplay-facing API changes are intended; the runtime shell remains responsible for texture ownership and metadata resolution.
