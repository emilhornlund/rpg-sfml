## Why

The overworld already stores terrain by chunk, but frame rendering still walks every tile in the configured world and performs per-tile lookups even when only a small portion of the map is on screen. As world sizes grow, that full-world render loop becomes avoidable work and cuts against the chunk-oriented design already present in `World`.

## What Changes

- Add a world-facing render query that derives visible terrain from the active camera frame instead of requiring `Game` to loop over the entire world grid every frame.
- Update overworld rendering so frame drawing visits only chunks and tiles that intersect the visible camera region, with a small overscan to avoid edge pop-in.
- Keep visible-terrain traversal aligned with retained chunk data so future chunk streaming or chunk-level render batching can build on the same boundary.
- Add focused specs and tests for camera-bounded terrain traversal, partial edge chunks, and preserving current tile visuals and player rendering.

## Capabilities

### New Capabilities
- `visible-chunk-rendering`: Define camera-bounded chunk/tile traversal for overworld rendering.

### Modified Capabilities
- `overworld-vertical-slice`: Change overworld rendering requirements so frames are produced from camera-visible terrain traversal instead of full-world loops.
- `world-chunk-caching`: Extend chunk-retention requirements to cover visible rendering queries reusing retained chunk data.

## Impact

- Affected specs: `openspec/specs/overworld-vertical-slice/spec.md`, `openspec/specs/world-chunk-caching/spec.md`, and new `openspec/specs/visible-chunk-rendering/spec.md`
- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, `src/main/Game.cpp`, and render-focused tests under `tests/`
- No new external dependencies or public executable entry point changes
