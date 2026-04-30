## 1. Extract generation logic

- [x] 1.1 Add a dedicated private world-generation helper/module under `src/main/` that accepts `WorldConfig` and produces generated tiles plus the spawn tile
- [x] 1.2 Move deterministic terrain classification, tile-grid construction, and spawn selection logic out of `World.cpp` into the helper/module without changing the generation results for the same configuration

## 2. Wire World through the generator boundary

- [x] 2.1 Update `World` construction to initialize its stored world state from the helper/module result while keeping the existing public API and world-facing queries unchanged
- [x] 2.2 Update main and test build wiring so the new helper/module is compiled everywhere `World` is built

## 3. Preserve behavior and boundary coverage

- [x] 3.1 Update or extend tests to cover the extracted generator boundary through the existing deterministic world, spawn, and traversability expectations
- [x] 3.2 Run the documented build and test commands and confirm the extracted generation path preserves the existing overworld behavior
