## 1. Catalog-driven runtime path resolution

- [x] 1.1 Extend `TilesetAssetLoader` to parse and expose the catalog runtime image field alongside authored source metadata.
- [x] 1.2 Update runtime tileset bootstrap code to load terrain and vegetation textures from catalog-resolved runtime image paths instead of hardcoded filename helpers.
- [x] 1.3 Move explicit non-catalog runtime path helpers to the consolidated `output/` layout, including the player spritesheet path.

## 2. Build staging alignment

- [x] 2.1 Update `src/main/CMakeLists.txt` to stage runtime tilesets, catalogs, spritesheets, and fonts from `assets/output/` into the executable-local `assets/output/` tree.
- [x] 2.2 Remove legacy staging assumptions that copy runtime PNGs or spritesheets from root-level asset paths.

## 3. Verification updates

- [x] 3.1 Update asset path and loader tests to assert the new `output/tilesets`, `output/spritesheets`, and catalog runtime image behavior.
- [x] 3.2 Update asset staging checks to validate the executable-local `assets/output/` directory structure.
- [x] 3.3 Run the existing build and test commands and resolve any regressions caused by the runtime asset path migration.
