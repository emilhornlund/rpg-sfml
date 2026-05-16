## Why

Terrain autotiling still derives neighbor context from the currently visible tile set, so tiles near the viewport edge can resolve the wrong transitions when off-screen world neighbors are not present in the render snapshot. The road pipeline also has enough stamping infrastructure to support richer layouts, but its topology vocabulary is still too coarse to express trails, main roads, plazas, dead ends, corners, tees, and crosses as first-class deterministic structure.

## What Changes

- Make overworld terrain and road rendering consume world-backed neighbor context instead of inferring missing neighbors from the currently visible snapshot contents.
- Extend the gameplay-owned render snapshot boundary so visible tiles and overlays can carry the structural neighbor information needed to render deterministically at viewport edges.
- Refine deterministic road topology semantics by classifying segments as trail, road, or main road.
- Refine deterministic road topology semantics by distinguishing node shapes such as dead ends, corners, tees, crosses, and plazas from gameplay anchors like spawn and destination.
- Feed the richer road semantics into the existing road-stamping pipeline so generation can create more interesting local structure without requiring new atlas roles or new art assets.

## Capabilities

### New Capabilities
- None.

### Modified Capabilities
- `render-snapshots`: published render snapshot entries will provide world-backed neighbor context needed for deterministic terrain and road rendering at viewport edges.
- `road-network-topology`: deterministic road topology will classify segment importance and node topology with richer structural semantics.
- `road-stamping`: stamping will consume the richer segment and node semantics when constructing shared structural road data.
- `overworld-road-overlays`: road overlay rendering will rely on world-backed structural neighbors and richer stamped semantics so motifs remain stable at viewport edges and across more interesting generated layouts.

## Impact

- Affected code: `include/main/World.hpp`, `include/main/OverworldRuntime.hpp`, `src/main/World.cpp`, `src/main/OverworldRuntime.cpp`, `src/main/GameRenderBatchSupport.cpp`, `src/main/RoadNetworkSupport.hpp`, `src/main/RoadStampSupport.hpp`, and related tests.
- Affected systems: overworld render snapshot publication, terrain autotiling, road topology generation, road stamping, and road overlay rendering.
- Dependencies: no new external dependencies or asset requirements; the change reuses the current atlas and metadata pipeline.
