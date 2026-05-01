## 1. Terrain tileset assets and wiring

- [x] 1.1 Add the initial four-cell overworld terrain tileset asset to the repository in a runtime-accessible location.
- [x] 1.2 Update executable build/resource wiring so the terrain tileset is staged to a stable path the game runtime can load.

## 2. Tile-size baseline and terrain rendering

- [x] 2.1 Change the overworld tile-size baseline from 32 to 16 world units and update tile-size-driven snapshot geometry and marker sizing accordingly.
- [x] 2.2 Add render-shell terrain tileset loading and fixed `TileType`-to-atlas-cell mapping in the SFML-facing game runtime.
- [x] 2.3 Replace terrain rectangle fill rendering with sprite-based tileset drawing while keeping marker rendering unchanged.

## 3. Verification

- [x] 3.1 Update or add tests that protect the existing gameplay/render boundary while covering both the 16-unit tile baseline and the new terrain tileset integration expectations.
- [x] 3.2 Build the executable and run the existing test suite with the tileset-backed terrain path enabled.
