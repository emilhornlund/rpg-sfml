## Why

The assets submodule now publishes generated tileset artifacts as catalogs under `output/catalogs/` with `*-catalog.json` filenames. The main repository still stages, loads, and tests against the old classification-based paths, so the current runtime asset layout no longer matches the checked-in asset outputs.

## What Changes

- **BREAKING** Rename runtime asset references from `output/classifications/` and `*-classification.json` to `output/catalogs/` and `*-catalog.json` wherever the main repository treats those files as emitted asset artifacts.
- Update build staging, executable asset lookup, and asset-dependent checks so they consume the renamed catalog outputs from the repository-root `assets/` tree and stage them into the built executable layout.
- Preserve the existing wrapped JSON schema and semantic tile metadata, including terrain `class` values and internal runtime classification concepts, while adopting catalog terminology for emitted files and paths.

## Capabilities

### New Capabilities

### Modified Capabilities

- `runtime-asset-layout`: Runtime staging and asset-dependent checks now use catalog-based generated asset paths and filenames.
- `tileset-asset-loading`: Runtime tileset loading now resolves catalog documents from staged assets while preserving the existing wrapped schema and semantic tile metadata.

## Impact

- `src/main/CMakeLists.txt`
- `src/main/GameAssetSupport.hpp`
- `src/main/TilesetAssetLoader.*`
- `src/main/TerrainAutotileSupport.*`
- `src/main/VegetationAtlasSupport.*`
- `tests/CMakeLists.txt`
- `tests/GameAssetSupportTests.cpp`
- `tests/TerrainAutotileSupportTests.cpp`
- `tests/TilesetAssetLoaderTests.cpp`
- `tests/check_main_asset_staging.cmake`
