## 1. Streaming terrain contract

- [x] 1.1 Refactor `WorldConfig` and world-facing assumptions so deterministic generation inputs no longer define a hard overworld boundary for terrain queries.
- [x] 1.2 Remove border-water and edge-falloff dependence on configured world extents so tile classification is derived from seed plus absolute tile coordinates.
- [x] 1.3 Update spawn selection to search from a deterministic anchor near the world origin instead of the center of a finite map.

## 2. On-demand chunk loading

- [x] 2.1 Refactor `World` chunk storage so missing chunks are generated and retained the first time tile queries reach them.
- [x] 2.2 Update visible terrain traversal to ensure required chunks exist for the camera-visible region plus overscan instead of clipping to precomputed world bounds.
- [x] 2.3 Keep repeated terrain and render queries reusing retained chunk data without regenerating existing chunks.

## 3. Runtime and test alignment

- [x] 3.1 Adjust runtime-facing world behavior so player movement and camera-driven rendering operate correctly with streaming terrain beyond the initial player vicinity.
- [x] 3.2 Replace finite-world tests that assume border water, fixed world edges, or eager full-world chunk population with streaming-world expectations.
- [x] 3.3 Add focused tests for deterministic chunk generation beyond the initial area, retained chunk reuse, and origin-anchored spawn validity.
