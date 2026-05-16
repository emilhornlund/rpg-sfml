## 1. Shared road occupancy

- [x] 1.1 Extract the duplicated road-membership logic from `World.cpp` and `WorldContent.cpp` into a shared helper boundary that both visibility and content placement can call.
- [x] 1.2 Replace the one-tile line test with deterministic widened road occupancy that produces at least two-tile-wide road arms and deterministic three-tile widening segments.
- [x] 1.3 Add or update world-facing tests that verify repeated road queries stay deterministic and that widened road occupancy stays aligned with road-aware content exclusion.

## 2. Road overlay metadata and selection

- [x] 2.1 Update `RoadOverlayTilesetSupport` to load and expose separate road overlay pools for `base`, `decor`, and `transition` tiles from the staged ground overlay catalog.
- [x] 2.2 Add deterministic interior road appearance selection that keeps transition tiles role-driven while choosing sparse `decor` replacements only for non-transition interior road tiles.
- [x] 2.3 Refresh road overlay metadata tests so they assert the new base/decor/transition semantics instead of treating all non-transition variants as base tiles.

## 3. Rendering behavior and motif coverage

- [x] 3.1 Update road overlay batch selection to use the widened occupancy patterns to compose edges, corners, end caps, and widened shoulders from existing transition roles.
- [x] 3.2 Add or update render-batch tests that cover surface-specific transitions, stable interior base selection, sparse decor selection, and composition of end-cap-like motifs.
- [x] 3.3 Run the existing build and relevant tests for road overlays, render batching, and overworld runtime behavior, then fix any regressions caused by the widened-road change.
