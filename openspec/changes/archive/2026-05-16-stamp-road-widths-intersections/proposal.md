## Why

The current road pipeline jumps directly from deterministic road topology to boolean tile occupancy, which makes widths, shoulders, curves, and intersections emerge from hard-coded footprint overlap instead of explicit structure. Adding a dedicated stamping layer lets the world own where roads go while a separate pass turns segment metadata into readable road shapes before autotile rendering selects the final visuals.

## What Changes

- Add a deterministic road-stamping stage between road topology generation and road overlay rendering.
- Extend road topology data with segment- and node-level metadata that can drive width rules, shoulders, curves, endpoint caps, and intersection treatment.
- Publish stamped road structure as the shared source of truth for road-covered tiles and structural connectivity instead of deriving all behavior from boolean segment occupancy.
- Update overworld road overlay rendering so autotile selection remains the final visual pass over stamped road structure rather than the stage that infers road shape.
- Keep world-owned road placement separate from rasterization so future road classes and layout rules can evolve without rewriting renderer-facing logic.

## Capabilities

### New Capabilities
- `road-stamping`: Define how deterministic road topology is stamped into tile-space structure with width rules, shoulders, curves, and intersections before visual autotiling.

### Modified Capabilities
- `road-network-topology`: Expand topology requirements so road nodes and segments carry structural metadata used by downstream stamping instead of acting as the final rasterized footprint.
- `overworld-road-overlays`: Change road overlay publication and rendering requirements to consume stamped road structure while keeping autotile selection as the final visual pass.

## Impact

- Affected specs: `road-network-topology`, `overworld-road-overlays`, and new capability `road-stamping`
- Likely affected code: `src/main/RoadNetworkSupport.hpp`, world-owned road publication paths in `src/main/World.cpp`, road-aware content queries in `src/main/WorldContent.cpp`, and render selection in `src/main/GameRenderBatchSupport.cpp`
- No external dependencies are expected, but deterministic tests will need broader coverage for stamped widths, intersections, and renderer-facing neighbor masks
