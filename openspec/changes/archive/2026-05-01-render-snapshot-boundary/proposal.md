## Why

The overworld gameplay side already computes most frame data, but `Game` still derives presentation details ad hoc while rendering. A dedicated render snapshot boundary makes the gameplay/runtime contract explicit so gameplay can publish a complete frame description and `Game` can stay focused on SFML-facing drawing.

## What Changes

- Introduce a gameplay-owned render snapshot that packages the active camera frame, visible terrain tiles, and marker data needed to draw the current overworld frame.
- Update the overworld runtime boundary so snapshot assembly happens within gameplay-facing runtime code instead of inside `Game`'s render path.
- Refactor `Game` to consume render snapshots as a thin renderer rather than deriving gameplay presentation state from runtime internals.
- Preserve the existing separation where SFML window ownership and backend drawing stay in `Game`, while world/player/camera collaboration stays behind the overworld runtime boundary.

## Capabilities

### New Capabilities
- `render-snapshots`: Define the gameplay-owned render snapshot contract for overworld frames, including visible tiles, camera framing, and marker data for renderable entities.

### Modified Capabilities
- `game-runtime`: Change the runtime shell requirement so `Game` renders snapshots supplied by the gameplay runtime boundary instead of assembling overworld presentation state ad hoc during drawing.

## Impact

- Affected code: `src/main/Game.cpp`, `src/main/OverworldRuntime.cpp`, `include/main/OverworldRuntime.hpp`, and supporting overworld/runtime types.
- Affected tests: runtime-focused tests covering snapshot assembly and the `Game`/overworld runtime rendering handoff.
- Affected systems: overworld runtime presentation flow and the contract between gameplay-owned state and the SFML renderer.
