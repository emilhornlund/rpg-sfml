## Why

The overworld currently generates from fixed compile-time constants, which makes the generation path deterministic but not configurable. Introducing explicit world generation inputs now turns the prototype into a reusable generation system and creates a stable seam for future work on alternate seeds, world sizes, and chunked procedural loading.

## What Changes

- Add a `World` configuration input that defines the generation seed, tile-grid width, tile-grid height, and tile size.
- Update overworld generation behavior so worlds generated with the same configuration remain deterministic while different configurations can produce different layouts and dimensions.
- Keep spawn selection, traversability checks, movement, camera framing, and rendering aligned with the configured world dimensions and tile size.
- Preserve the existing default overworld behavior by providing a built-in default configuration for current runtime startup.

## Capabilities

### New Capabilities
- None.

### Modified Capabilities
- `overworld-vertical-slice`: Change overworld generation from fixed built-in constants to configurable world inputs while preserving deterministic generation for a given configuration.

## Impact

- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, `src/main/Game.cpp`, and overworld-focused tests.
- Affected API: `World` construction and any runtime code that assumes hardcoded world dimensions or tile size.
- Dependencies: No new external dependencies; change stays within the existing gameplay module and runtime boundaries.
