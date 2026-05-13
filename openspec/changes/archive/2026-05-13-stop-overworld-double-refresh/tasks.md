## 1. Runtime refactor

- [x] 1.1 Split one-time overworld session setup from snapshot publication inside `OverworldRuntime` so `update()` no longer routes through the full `initialize()` path.
- [x] 1.2 Preserve `initialize()` as the public entry point that publishes an initial render/debug snapshot for the provided viewport while making `update()` publish the active frame through a single refresh pass.

## 2. Regression coverage

- [x] 2.1 Update or add focused `OverworldRuntime` tests that keep the initialization snapshot contract intact after the refactor.
- [x] 2.2 Add coverage that exercises `update()`-driven snapshot publication for debug zoom and viewport changes without requiring an extra refresh path.

## 3. Validation

- [x] 3.1 Build the project and run the existing test suite to confirm the runtime refactor preserves current behavior.
