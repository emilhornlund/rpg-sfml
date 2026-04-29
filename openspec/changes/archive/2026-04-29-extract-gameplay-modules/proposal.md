## Why

`Game.cpp` currently defines placeholder `World`, `Player`, and `Camera` types inline, which makes the runtime coordinator responsible for both orchestration and gameplay domain ownership. Extracting those types into dedicated files creates a clearer gameplay boundary now, before real world, player, and camera logic starts accumulating in the wrong place.

## What Changes

- Extract `World`, `Player`, and `Camera` into dedicated runtime-facing module types with their own headers and source files.
- Update `Game` to own and coordinate those modules instead of declaring gameplay placeholders inline in `Game.cpp`.
- Establish module responsibilities and construction boundaries so future gameplay logic can grow behind those types without expanding the top-level loop coordinator.

## Capabilities

### New Capabilities
- `gameplay-modules`: Define dedicated gameplay module types and file boundaries for world, player, and camera coordination within the runtime.

### Modified Capabilities
- `game-runtime`: Tighten the runtime coordination requirements so `Game` remains an orchestration layer instead of housing gameplay type definitions directly.

## Impact

- Affected code: `src/main/Game.cpp`, `include/main/`, `src/main/`, and related build/test wiring.
- Affected design: runtime ownership and file organization for world, player, and camera collaborators.
- No external API or dependency changes are expected.
