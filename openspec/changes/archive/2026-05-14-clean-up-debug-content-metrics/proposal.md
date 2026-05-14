## Why

The debug overlay currently reports both rendered generated-content count and visible generated-content count even though both values are populated from the same source for every frame. This makes the overlay imply a distinction that the runtime does not actually compute, which weakens the usefulness of the diagnostics.

## What Changes

- Remove the duplicated generated-content diagnostic from the debug overlay so each published metric has distinct meaning.
- Simplify the gameplay-owned debug snapshot implementation so it only carries the generated-content visibility metric that the runtime actually computes today.
- Update overlay-facing requirements and tests to describe a single visible generated-content metric instead of two equivalent counters.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `debug-overlay-display`: Change the overlay diagnostics contract to stop presenting separate rendered and visible generated-content counts when the runtime does not distinguish them.

## Impact

- Affected specs: `openspec/specs/debug-overlay-display/spec.md`
- Affected code: `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, `src/main/GameRuntimeSupport.cpp`
- Affected tests: `tests/OverworldRuntimeTests.cpp`, `tests/GameRuntimeSupportTests.cpp`, and any other overlay-string expectations
