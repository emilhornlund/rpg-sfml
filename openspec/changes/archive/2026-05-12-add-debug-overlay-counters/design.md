## Context

`buildDebugOverlayString(...)` currently reports FPS, retained chunk count, retained generated-content count, rendered generated-content count, player coordinates, and zoom level. That helps with general state inspection, but it does not show the quantities most likely to scale with the recent terrain and grid rendering work: visible tiles, visible generated content, front-layer occluders, and the terrain/grid vertex counts built in `Game::render()`.

This change needs to improve observability without weakening the gameplay/render boundary. `OverworldRuntime` already owns gameplay-derived debug metrics, while `Game::render()` owns front-occluder classification and SFML vertex-array construction. The new counters should follow that same ownership split.

## Goals / Non-Goals

**Goals:**
- Make the overlay show the frame-local counts that best explain large-scene rendering cost.
- Keep gameplay-owned counts inside `OverworldDebugSnapshot`.
- Keep render-only counters inside the render shell where they are computed.
- Preserve the existing overlay layout and snapshot-driven boundary.

**Non-Goals:**
- Add profiling timers, historical graphs, or persistent performance logging.
- Move SFML vertex-array details into gameplay modules.
- Change rendering behavior, batching strategy, or debug controls beyond the new diagnostics.

## Decisions

### Decision: Publish visibility counts from `OverworldDebugSnapshot`

The debug snapshot will add visible tile count and visible generated-content count, sourced directly from `m_renderSnapshot.visibleTiles.size()` and `m_renderSnapshot.generatedContent.size()` in `OverworldRuntime::refreshRenderSnapshot()`.

**Why this over computing them in `Game::render()`?**
- Those counts are gameplay-owned snapshot facts that already exist before shell-side rendering begins.
- Publishing them keeps the overlay fully snapshot-driven for runtime-derived data.

**Alternative considered:** have `Game` read the render snapshot collections and compute the counts ad hoc for the overlay. Rejected because the existing debug snapshot already serves as the runtime-owned diagnostic contract.

### Decision: Keep occluder and vertex counters render-side

Front occluder count, terrain vertex count, and tile-grid vertex count will remain render-shell diagnostics because they only exist after `Game::render()` builds the queue and vertex arrays.

**Why this over extending `OverworldDebugSnapshot` with all counters?**
- `frontOccluderIndices` and SFML vertex counts are implementation details of the shell-owned render path, not gameplay state.
- Keeping them render-side preserves the boundary established by the existing batching work.

**Alternative considered:** back-propagate render-shell counts into the runtime before building the overlay string. Rejected because it would mix shell diagnostics into gameplay-owned snapshot types.

### Decision: Pass render-only metrics to the overlay builder through a small helper struct

The overlay formatter will consume the gameplay debug snapshot plus a small render-side metrics struct for front occluders and vertex counts.

**Why this over extending the formatter with multiple loose integer parameters?**
- A dedicated struct keeps the expanded overlay input readable and easy to extend.
- It makes the render-owned versus gameplay-owned split explicit at the call site.

**Alternative considered:** build the overlay text inline in `Game::render()`. Rejected because the project already centralizes overlay-string formatting in `GameRuntimeSupport.hpp`.

### Decision: Report tile-grid vertex count as zero when the grid overlay is disabled

The overlay will always show the grid vertex counter, with a value of zero when tile-grid rendering is off.

**Why this over hiding the line when disabled?**
- A stable overlay shape is easier to scan while comparing frames.
- Zero communicates both "currently disabled" and "no grid vertices were emitted" without extra conditional formatting.

**Alternative considered:** omit the grid-vertex line entirely when the grid is disabled. Rejected because it makes the overlay harder to compare frame to frame.

## Risks / Trade-offs

- **[More overlay lines make the panel taller]** → Mitigation: keep the new counters concise and preserve the existing single-column text layout.
- **[Render-side counters could be mistaken for gameplay snapshot data]** → Mitigation: keep only visibility counts in `OverworldDebugSnapshot` and group render-owned values in a separate shell-side input struct.
- **[Grid vertex count semantics could be ambiguous while the overlay is disabled]** → Mitigation: specify that the count is zero when no grid vertex array is emitted.

## Migration Plan

1. Extend the debug snapshot contract with the two visibility counts.
2. Thread render-only counts from `Game::render()` into the overlay formatter.
3. Update overlay and runtime tests to cover the expanded diagnostics.
4. Rollback is a straightforward reversion because no external interfaces or asset formats change.

## Open Questions

- No blocking design questions remain.
