## 1. World content model

- [x] 1.1 Replace the minimal world content record types in `include/main/World.hpp` with data-only content structs for semantic type, content instance data, chunk content, world position, footprint, and appearance id.
- [x] 1.2 Update retained chunk storage and world-facing query signatures so `World` returns the richer chunk content shape without leaking implementation details outside the module boundary.

## 2. Deterministic content generation

- [x] 2.1 Update `src/main/WorldContent.hpp` and `src/main/WorldContent.cpp` so chunk content generation produces deterministic content instances with stable ids, world-space placement, footprints, and appearance identifiers.
- [x] 2.2 Keep unsupported chunks returning empty content while ensuring deterministic chunk ownership and instance data for repeated queries and matching worlds.

## 3. Verification

- [x] 3.1 Update content-focused tests in `tests/WorldTerrainGeneratorTests.cpp` and `tests/OverworldVerticalSliceTests.cpp` to validate the new chunk content structure and deterministic instance fields.
- [x] 3.2 Run the documented build and test commands to confirm the richer data-only content model integrates cleanly with the existing world module.
