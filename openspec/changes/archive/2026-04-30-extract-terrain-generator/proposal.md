## Why

The `World` module currently owns both overworld runtime state and the concrete terrain generation rules used to build that state. Extracting generation into a dedicated helper keeps `World` focused on world-facing queries and makes future terrain evolution easier without growing the runtime module boundary in the wrong direction.

## What Changes

- Extract deterministic terrain layout generation and spawn selection from `World.cpp` into a dedicated world-generation helper/module.
- Keep `World` responsible for owning generated state and answering world-facing runtime queries.
- Preserve the existing deterministic overworld behavior, including configured dimensions, tile classification results, traversability, and valid spawn selection.
- Update tests and implementation wiring so future terrain-rule changes can happen behind the generator boundary instead of inside `World`.

## Capabilities

### New Capabilities
<!-- None. -->

### Modified Capabilities
- `gameplay-modules`: clarify that world-specific generation rules can be delegated to dedicated world collaborators so `World` remains the stable gameplay-facing boundary instead of accumulating procedural generation details.

## Impact

- Affected specs: `openspec/specs/gameplay-modules/spec.md`
- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, `src/main/CMakeLists.txt`, `tests/CMakeLists.txt`, and new private world-generation helper files under `src/main/`
- No expected public API changes for `Game`, `Player`, `Camera`, or `World` consumers
