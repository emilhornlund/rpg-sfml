## 1. Road network model

- [x] 1.1 Add an internal deterministic road-network data model for nodes, segments, and destination-oriented anchors within the world boundary.
- [x] 1.2 Implement seeded road-network generation anchored at spawn that can produce straight connections, branches, loops, and endpoints.
- [x] 1.3 Add focused tests for deterministic topology generation and spawn-connected network invariants.

## 2. Topology-backed occupancy

- [x] 2.1 Replace spawn-cross road occupancy logic in the shared road helper with topology-backed segment, junction, endpoint, and destination footprint evaluation.
- [x] 2.2 Update `World` road queries and visible road overlay publication to consume the new shared topology-backed occupancy path.
- [x] 2.3 Update `WorldContent` road-aware exclusion to use the same topology-backed occupancy decisions as visible road publication.

## 3. Rendering and verification

- [x] 3.1 Refresh road overlay tests to cover topology-driven motifs such as branches, loop bends, and explicit endpoints without graph-specific render roles.
- [x] 3.2 Re-evaluate published-road clipping rules so valid topology-authored endpoints are preserved while invalid single-tile artifacts remain suppressed.
- [x] 3.3 Run the documented build and test commands and resolve any failures caused by the new road-network behavior.
