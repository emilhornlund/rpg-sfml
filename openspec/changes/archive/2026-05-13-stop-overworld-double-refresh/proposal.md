## Why

`OverworldRuntime::update()` currently routes through `initialize()`, even though `initialize()` already updates the camera and rebuilds the render and debug snapshots. `update()` then repeats that same camera and snapshot work again after advancing the player, causing duplicate world queries and snapshot rebuilds on every frame.

## What Changes

- Separate one-time session setup from per-frame snapshot publication inside `OverworldRuntime`.
- Keep `initialize()` responsible for publishing an initial render-ready snapshot for the provided viewport.
- Change `update()` so it performs at most one camera update and one snapshot refresh for the active frame while preserving the existing published render and debug snapshot behavior.
- Add or update focused tests that protect the initialization contract and the single-refresh-per-frame update flow.

## Capabilities

### New Capabilities
<!-- None. -->

### Modified Capabilities
- `render-snapshots`: tighten the overworld snapshot publication requirement so initialization still publishes a snapshot, while each frame update publishes the current frame snapshot without redundant refresh work.

## Impact

- Affected code: `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, and `tests/OverworldRuntimeTests.cpp`
- Affected behavior: overworld snapshot publication sequencing during initialization and frame updates
- Affected systems: overworld runtime orchestration, camera framing, world visibility queries, render/debug snapshot assembly
