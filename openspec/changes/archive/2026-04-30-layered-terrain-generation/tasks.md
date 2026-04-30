## 1. Terrain signal generation

- [x] 1.1 Replace the current per-tile hash-threshold classifier in `WorldTerrainGenerator.cpp` with deterministic layered elevation evaluation that preserves a water border for the finite map.
- [x] 1.2 Add a separate deterministic moisture signal and normalize both signals so they can be used consistently across the full configured world size.
- [x] 1.3 Implement tile classification rules that map elevation and moisture into the existing `Water`, `Sand`, `Grass`, and `Forest` categories.

## 2. World behavior integration

- [x] 2.1 Keep generated tile storage, traversability, and spawn selection wired through the existing `GeneratedWorldData` and `World` flow without expanding the public `World` API.
- [x] 2.2 Tune shoreline and inland thresholds so coastlines, grasslands, and forests emerge from the new layered rules while preserving a playable traversable interior.

## 3. Tests and verification

- [x] 3.1 Update terrain-generation tests to assert deterministic output, water borders, traversable spawn selection, and the revised biome-mapping behavior.
- [x] 3.2 Update any higher-level overworld tests that depend on terrain layout assumptions so they validate the new signal-driven generation contract rather than the old random-threshold behavior.
- [x] 3.3 Run the documented build and test commands and resolve any failures introduced by the terrain-generation changes.
