## 1. World helper retention

- [x] 1.1 Update `World` state ownership so a world instance retains its deterministic terrain and content generation helpers without creating a header cycle.
- [x] 1.2 Rewire `World` construction and `ensureChunkRetained()` to initialize those helpers once per world instance and reuse them for spawn generation and missing chunk loads.

## 2. Regression coverage

- [x] 2.1 Extend world-generation tests to verify repeated missing chunk loads in the same `World` instance reuse retained generation helpers while preserving deterministic chunk results.
- [x] 2.2 Run the existing CMake build and CTest suite to confirm the refactor keeps chunk caching, unloading, and regeneration behavior unchanged.
