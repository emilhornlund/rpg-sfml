## 1. Chunk coordinate foundations

- [x] 1.1 Add world-owned chunk coordinate and chunk-local coordinate helpers that map absolute tile coordinates with floor-division semantics, including negative tile positions.
- [x] 1.2 Introduce chunk-oriented terrain data structures behind `World` and update world state to retain generated chunks instead of a single eager tile array.

## 2. Chunk-backed terrain generation

- [x] 2.1 Refactor `WorldTerrainGenerator` so terrain classification can generate chunk data from absolute tile coordinates and chunk-local indexing rather than one full finite grid pass.
- [x] 2.2 Update `World` tile lookup, traversability, and spawn selection paths to resolve terrain through the chunk cache while preserving the current gameplay-facing world behavior.

## 3. Runtime alignment and coverage

- [x] 3.1 Adjust runtime and rendering assumptions that directly depend on eagerly stored world tiles so the current overworld slice still works with chunk-backed world queries.
- [x] 3.2 Add or update focused tests for deterministic chunk-backed lookup, negative chunk boundary mapping, chunk cache reuse, and preserved overworld slice behavior.
