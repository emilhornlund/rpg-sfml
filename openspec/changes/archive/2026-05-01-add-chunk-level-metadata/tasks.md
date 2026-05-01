## 1. Chunk metadata model

- [x] 1.1 Introduce world-owned chunk metadata types and replace the retained chunk tile vector with a structured chunk record behind `World`.
- [x] 1.2 Extend `TerrainGenerator::generateChunk()` to derive biome summary, traversability summary, and deterministic candidate locations in the same pass that builds tile content.
- [x] 1.3 Keep candidate generation limited to traversable tiles and preserve current deterministic behavior for repeated chunk generation.

## 2. World retention and query flow

- [x] 2.1 Update chunk retention helpers in `World.cpp` so missing chunks are generated once and reused for tile, metadata, and visible rendering queries.
- [x] 2.2 Add world-facing metadata query access that resolves chunk coordinates through `World` without exposing retained chunk storage directly.
- [x] 2.3 Ensure player spawn behavior remains deterministic and traversable while the new metadata is introduced.

## 3. Validation

- [x] 3.1 Add or update tests covering deterministic chunk metadata for repeated queries and matching world instances.
- [x] 3.2 Add or update tests covering candidate validity, including empty candidates for unsuitable chunks and traversable-only candidate placement.
- [x] 3.3 Run the existing build and test commands to confirm the chunk metadata changes integrate cleanly.
