## Why

The current road overlay source of truth is a spawn-centered widened cross, which keeps rendering deterministic but prevents the world from expressing branches, loops, and destination-oriented roads. Replacing that shape logic with a small world-owned road network lets the existing overlay renderer compose richer road motifs from real topology instead of from hard-coded band math.

## What Changes

- Introduce a deterministic road-network model owned by `World`, with graph-like road nodes and segments anchored around spawn and nearby destinations.
- Derive road occupancy queries from that road network so `World::hasRoadOverlay()`, visible road publication, and road-aware vegetation exclusion all use the same topology-backed source of truth.
- Expand deterministic road layout behavior from the current plus-shaped spawn road into a small network that can include branches, loops, segment endpoints, and optional district-center destinations.
- Keep road rendering neighbor-driven and surface-aware, so the existing road overlay autotile system continues to resolve transitions, corners, and end-cap motifs from local occupancy.

## Capabilities

### New Capabilities
- `road-network-topology`: deterministic world-owned road graph data with nodes, segments, endpoints, and destination anchors suitable for occupancy queries

### Modified Capabilities
- `overworld-road-overlays`: road overlay occupancy changes from spawn-centered band logic to road-network-backed occupancy that supports branches, loops, and destination-connected roads

## Impact

- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, `src/main/WorldContent.*`, `src/main/RoadOverlayWorldSupport.hpp`, `src/main/OverworldRuntime.cpp`, and relevant road/world tests
- Affected behavior: visible road overlay shapes, road-aware content exclusion, and the world-owned contract for deterministic road occupancy
- Dependencies: no new external dependencies; change stays within the current world/runtime/render boundaries
