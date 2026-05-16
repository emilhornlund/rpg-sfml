## 1. Topology metadata

- [x] 1.1 Extend road topology types with deterministic segment and node metadata for width, shoulders, and junction treatment
- [x] 1.2 Update topology construction helpers so generated road networks assign stable metadata alongside nodes, segments, and polylines
- [x] 1.3 Add focused tests for deterministic topology metadata generation across repeated builds

## 2. Road stamping

- [x] 2.1 Introduce a world-owned road stamping helper that converts topology into tile-space structural road data
- [x] 2.2 Implement deterministic stamping rules for corridor width, shoulders, bends, endpoints, and intersections
- [x] 2.3 Add tests covering stamped road continuity, width changes, bends, and junction shapes

## 3. World integration

- [x] 3.1 Update visible road overlay publication to consume stamped road results instead of direct footprint occupancy helpers
- [x] 3.2 Update road-aware content exclusion to use the same stamped road source of truth
- [x] 3.3 Remove or narrow legacy direct occupancy helpers once all world-owned callers use the stamped path

## 4. Rendering and verification

- [x] 4.1 Update road overlay render selection to resolve atlas roles from stamped local structure while preserving deterministic base and decor selection
- [x] 4.2 Extend render-batch and road-overlay tests for widths, curves, shoulders, and stamped intersections
- [x] 4.3 Run the existing build and test commands to confirm the stamped road pipeline integrates cleanly
