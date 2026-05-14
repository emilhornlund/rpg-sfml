## 1. Define the world-facing chunk inspection API

- [x] 1.1 Choose the concrete borrowed accessor shape for retained metadata and content inspection, and finalize the public naming for borrowed versus snapshot access.
- [x] 1.2 Document the borrowed-result lifetime and invalidation rules in the `World` public API comments.

## 2. Implement borrowed and snapshot chunk access

- [x] 2.1 Update `World` declarations and definitions so metadata inspection supports both a borrowed read-only path and an owned snapshot path.
- [x] 2.2 Update `World` declarations and definitions so content inspection supports both a borrowed read-only path and an owned snapshot path without copying on inspection-only reads.

## 3. Cover the contract with tests

- [x] 3.1 Update world query tests to verify borrowed metadata and content access returns retained chunk data without changing deterministic query behavior.
- [x] 3.2 Add or update tests that cover the snapshot path and the documented borrowed-result validity boundary around retained chunk eviction.
