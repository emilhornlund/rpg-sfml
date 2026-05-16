## Why

The asset repository now ships runtime-ready files under `assets/output/`, but the game runtime and build staging still assume root-level tileset PNGs and a root-level player spritesheet. That mismatch breaks the intended catalog-driven asset boundary and makes the executable depend on an outdated asset layout.

## What Changes

- Update runtime asset path resolution to consume staged runtime files from `assets/output/` instead of legacy root-level PNG locations.
- Make tileset loading use the catalog's runtime image metadata for terrain and vegetation atlases while preserving authored source metadata in the catalog.
- Move non-catalog runtime path helpers, build staging, and asset-dependent checks to the new `output/tilesets`, `output/spritesheets`, and existing `output/fonts` layout.
- Align repository specs and tests with the new staged runtime asset structure.

## Capabilities

### New Capabilities

- None.

### Modified Capabilities

- `runtime-asset-layout`: Change the staged executable asset requirements from mixed root-level files to the consolidated `assets/output/` runtime layout.
- `tileset-asset-loading`: Change runtime image resolution so wrapped tileset catalogs use explicit runtime image metadata instead of treating authored source image paths as runtime assets.

## Impact

- Affected code: `src/main/GameAssetSupport.hpp`, `src/main/GameResourceBootstrapSupport.cpp`, `src/main/TilesetAssetLoader.{hpp,cpp}`, `src/main/CMakeLists.txt`, and asset-dependent tests under `tests/`.
- Affected systems: executable asset staging, tileset catalog loading, runtime texture bootstrap, and asset layout checks.
- Affected behavior: runtime terrain and vegetation textures resolve from catalog-declared runtime PNGs under `assets/output/tilesets/`, and the player spritesheet resolves from `assets/output/spritesheets/`.
