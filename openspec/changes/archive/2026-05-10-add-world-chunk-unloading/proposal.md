## Why

The overworld currently retains every generated chunk it ever touches, so exploring new terrain causes loaded object counts and retained memory to grow monotonically. This makes streaming performance hard to reason about and turns the debug overlay into evidence of unbounded cache growth instead of healthy runtime behavior.

## What Changes

- Add a bounded world-chunk retention policy so chunks outside an active retention window can unload.
- Preserve deterministic terrain and content regeneration when an unloaded chunk is queried again later.
- Publish clearer streaming diagnostics so the debug overlay reflects currently retained cache state rather than an ever-growing historical total.
- Keep visible rendering stable by retaining the chunks needed for the active camera view and content overscan window.

## Capabilities

### New Capabilities
<!-- None. -->

### Modified Capabilities
- `world-chunk-caching`: change chunk retention from unbounded lifetime retention to bounded reuse with unloading outside a defined retention window.
- `render-snapshots`: expand debug snapshot streaming diagnostics so the shell can display cache state that matches the new bounded retention behavior.
- `debug-overlay-display`: update overlay diagnostics to present readable cache and visibility counts for streaming analysis.

## Impact

- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, `include/main/OverworldRuntime.hpp`, `src/main/OverworldRuntime.cpp`, `src/main/Game.cpp`, and related tests in `tests/`.
- Affected behavior: overworld chunk lifetime, generated-content retention totals, and debug overlay wording/metrics.
- No new external dependencies or public non-OpenSpec APIs are expected.
