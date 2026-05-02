## Why

`World` already retains deterministic generated chunk content, but `OverworldRuntime` only publishes terrain tiles and the player marker through the render snapshot. That leaves generated content behind the world boundary with no runtime-owned path into rendering, even though the snapshot is supposed to be the gameplay-owned source of frame presentation.

## What Changes

- Extend the overworld render snapshot contract so the runtime publishes generated chunk content alongside terrain and player data for the active camera frame.
- Add a render-facing data shape for generated world content so the game shell can draw deterministic content without querying `World` directly or reinterpreting chunk content records.
- Update overworld frame assembly rules so `OverworldRuntime` merges visible terrain, player presentation, and visible generated content into one render snapshot.
- Update the SFML shell-facing overworld rendering requirements so generated content is rendered from snapshot data rather than hidden behind world-only queries.

## Capabilities

### New Capabilities
- `generated-content-rendering`: Define render-facing publication and rendering expectations for deterministic generated world content in the overworld frame.

### Modified Capabilities
- `render-snapshots`: Expand snapshot requirements so visible generated content is published together with terrain and player data.
- `overworld-vertical-slice`: Expand the overworld frame and rendering requirements so generated content appears in the runtime-owned render flow.

## Impact

- Affected specs: `openspec/specs/render-snapshots/spec.md`, `openspec/specs/overworld-vertical-slice/spec.md`, and a new `openspec/specs/generated-content-rendering/spec.md`
- Likely affected code: `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, `include/main/World.hpp`, `src/main/World.cpp`, `src/main/Game.cpp`, and runtime/render tests
- No external dependencies or asset pipeline changes are required for the first pass
