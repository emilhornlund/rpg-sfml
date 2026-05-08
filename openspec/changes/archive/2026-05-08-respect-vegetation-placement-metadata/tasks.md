## 1. Preserve vegetation placement metadata

- [x] 1.1 Extend the tileset asset loading structures and parsing code to retain vegetation anchor placement metadata, including `placeOn` and `biomes`.
- [x] 1.2 Extend vegetation metadata assembly so each prototype exposes the loaded placement metadata alongside existing geometry and atlas-part data.

## 2. Use metadata-driven vegetation placement

- [x] 2.1 Replace the hardcoded terrain-specific vegetation prototype pools in world content generation with metadata-driven candidate selection grouped by supported anchor tile type.
- [x] 2.2 Update deterministic placement logic so eligible prototypes require matching `placeOn` support and positive biome weight for the anchor tile type, including valid water vegetation.
- [x] 2.3 Remove chunk-level gating that prevents fully-water chunks from producing valid metadata-qualified vegetation placements.

## 3. Verify deterministic behavior

- [x] 3.1 Add or update tests for vegetation metadata loading so runtime metadata preserves `placeOn` and `biomes` values for water and land prototypes.
- [x] 3.2 Add or update world content tests to confirm metadata-qualified water vegetation can appear while deterministic vegetation identities and repeated query stability remain intact.
