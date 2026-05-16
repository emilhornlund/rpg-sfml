## 1. Snapshot Neighbor Context

- [x] 1.1 Extend visible world and overworld render snapshot types to publish world-backed terrain neighbor categories and stamped road neighbor occupancy.
- [x] 1.2 Update `World` and `OverworldRuntime` to populate the new neighbor context from deterministic world queries for every visible tile and visible road overlay.
- [x] 1.3 Update render snapshot and runtime tests to cover stable neighbor publication, including viewport-edge cases.

## 2. Road Semantics and Stamping

- [x] 2.1 Refine road topology data so segment class is expressed as trail, road, or main road and node structure is expressed separately as dead end, corner, tee, cross, or plaza.
- [x] 2.2 Update the road generator and stamping helpers to consume the richer topology semantics while preserving deterministic occupancy and content-exclusion alignment.
- [x] 2.3 Add or update topology and stamping tests for semantic classification and stamped structural outcomes across representative junction shapes.

## 3. Render Batching and Overlay Resolution

- [x] 3.1 Replace visible-subset terrain and road neighbor inference in render batching with the world-backed neighbor context published in the render snapshot.
- [x] 3.2 Keep road overlay atlas selection surface-aware and motif-composed with the existing metadata while ensuring viewport-edge tiles resolve the same as fully visible tiles.
- [x] 3.3 Update render batch and scene tests to cover deterministic viewport-edge terrain transitions and road motifs without requiring new atlas roles.
