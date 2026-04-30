## Context

`World` currently hardcodes its seed, width, height, and tile size inside `World.cpp`. That keeps the starter slice simple, but it also means the generation logic cannot be reused with alternate world shapes or sizes without editing implementation constants. The change needs to introduce explicit configuration at the `World` boundary while preserving the current default startup behavior and the existing module split between `Game`, `World`, `Player`, and `Camera`.

## Goals / Non-Goals

**Goals:**
- Make world generation inputs explicit at the `World` API boundary.
- Preserve deterministic generation for identical configuration values.
- Keep existing runtime startup working through a built-in default configuration.
- Ensure spawn lookup, traversability, camera bounds, and rendering continue to derive from the generated world's configured dimensions and tile size.

**Non-Goals:**
- Changing the terrain classification algorithm beyond accepting a configurable seed.
- Introducing chunk streaming, save/load, or runtime world-regeneration UI.
- Making `Game` publicly configurable through `Game.hpp` in this change.

## Decisions

### Introduce a value-type world configuration

Add a small `WorldConfig` value type that contains seed, width in tiles, height in tiles, and tile size. This makes the generation contract explicit and keeps configuration immutable from the caller's perspective after construction.

**Why this over separate constructor parameters?**
- A value type keeps the API readable as the number of generation inputs grows.
- Tests can create named configurations without relying on argument ordering.
- Future generation options can be added without multiplying overloads.

### Keep a default-constructed world path

Retain a no-argument `World` construction path by routing it through a built-in default configuration equivalent to today's hardcoded behavior.

**Why this over requiring configuration everywhere immediately?**
- It preserves the current runtime startup and keeps the first increment small.
- Existing call sites and tests can move gradually to explicit configuration where helpful.

### Make derived world state depend on configuration, not globals

Generation, spawn selection, world-size calculations, coordinate conversion, and traversability checks should derive from the active `WorldConfig` stored in the world state instead of compile-time constants in `World.cpp`.

**Why this over partially replacing constants?**
- It removes hidden coupling between generation and runtime behaviors.
- Width, height, and tile size then stay coherent across movement, camera clamping, and rendering.

### Preserve deterministic behavior per configuration

The tile classifier should incorporate the configured seed while continuing to produce stable tile categories for the same coordinates and configuration.

**Why this over random-device-based generation?**
- Determinism is already part of the overworld contract and is useful for tests.
- The project can still support different worlds by varying the seed explicitly.

## Risks / Trade-offs

- **Configuration validation complexity** → Keep this change small by defining a narrow valid configuration contract and using safe defaults for the runtime path.
- **API growth in `World`** → Mitigate by introducing a single configuration type instead of many new accessors and overloads.
- **Spec drift between default and configured worlds** → Mitigate by defining that the default configuration preserves current behavior and by covering both configured and default paths in tests.
