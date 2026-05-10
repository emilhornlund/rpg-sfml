## Context

The current overworld cache keeps every generated chunk in `World::m_state.chunks` for the lifetime of the runtime. That makes terrain and vegetation queries fast for revisited areas, but it also means retained chunk data and retained generated-content counts only grow as the player explores. The debug overlay currently reports loaded generated-content count from that unbounded cache, which makes streaming behavior look unhealthy even when only a small region is actively visible.

This change crosses `World`, `OverworldRuntime`, render snapshots, the game shell overlay, and tests. It also changes performance behavior, so the design needs an explicit retention and eviction strategy before implementation starts.

## Goals / Non-Goals

**Goals:**
- Bound retained chunk memory during normal exploration.
- Preserve deterministic terrain and generated-content results when an evicted chunk is loaded again.
- Keep enough nearby chunks retained to avoid visible popping or per-frame regeneration churn around the camera.
- Publish debug diagnostics that clearly describe the current cache state for streaming analysis.

**Non-Goals:**
- Persisting runtime mutations to unloaded chunks.
- Adding asynchronous streaming, background generation, or disk-backed caches.
- Changing terrain or vegetation generation rules.
- Reworking the public gameplay module boundaries beyond the minimum retention controls needed for streaming.

## Decisions

### 1. World retention becomes bounded around the active frame

`World` will continue to generate missing chunks on demand, but retained chunk lifetime will no longer be unbounded. The runtime will establish an active retention window derived from the current camera frame plus the existing content overscan region, and `World` will prune retained chunks that fall outside a one-chunk margin buffer around that active window.

Using a one-chunk margin keeps the currently visible area and nearby traversal space warm while preventing exploration history from accumulating forever. For the current camera sizes, that gives enough spatial slack for normal movement and short backtracking without paying the higher steady-state memory cost of a two-chunk ring.

**Alternatives considered**
- **Never unload chunks**: simplest, but memory and loaded-object metrics grow monotonically.
- **LRU cache without spatial policy**: bounds memory, but can evict nearby chunks unpredictably and cause churn while moving back and forth.
- **Distance-based pruning from player tile only**: simpler than frame-based retention, but less aligned with actual rendering and large-content visibility rules.

### 2. Deterministic regeneration is the unload contract

Eviction will remove only retained derived data: tiles, metadata, and generated chunk content. When a future query touches an evicted chunk, the world will regenerate it through the existing deterministic terrain/content collaborators using the same generation inputs and chunk coordinates.

This keeps unloading compatible with the current world model because retained chunks are derived cache entries, not authoritative mutable gameplay state.

**Alternatives considered**
- **Persist evicted chunks in a secondary serialized cache**: unnecessary complexity for the current deterministic prototype.
- **Keep content retained longer than terrain**: complicates cache invariants and makes diagnostics harder to interpret.

### 3. Streaming diagnostics should describe current retained state

The debug snapshot will expose cache-oriented metrics that match the new bounded retention model. The overlay will show retained chunk count, retained generated-content count, and rendered generated-content count on separate lines, alongside existing player coordinates, zoom, and frame rate.

This makes the overlay useful for tuning retention behavior: the counts should rise while entering a new area, stabilize during local movement, and fall when chunks are evicted behind the player.

**Alternatives considered**
- **Keep only the existing loaded objects label**: ambiguous once unloading exists and still hides chunk-level cache behavior.
- **Expose only chunk count**: not enough to reason about generated-content footprint.

### 4. Retention pruning should run from the gameplay-owned frame refresh path

The pruning trigger should live in the same gameplay update path that already refreshes render snapshots. That keeps retention aligned with the active camera, avoids pushing streaming policy into the shell, and preserves the `Game -> OverworldRuntime -> World` ownership boundary.

**Alternatives considered**
- **Prune inside arbitrary world queries**: makes behavior harder to reason about and can cause surprising cache churn during helper/test access patterns.
- **Prune in the outer `Game` shell**: leaks gameplay retention policy across the runtime boundary.

## Risks / Trade-offs

- **[Risk] Edge churn near the retention boundary** → Mitigation: include a chunk-margin buffer beyond the visible/content overscan window instead of pruning exactly at the visible edge.
- **[Risk] Regeneration cost becomes visible during fast exploration** → Mitigation: keep the active window camera-based and buffered so common backtracking and near-edge movement reuse retained chunks.
- **[Risk] Future mutable world state would be incompatible with pure eviction** → Mitigation: keep this change scoped to deterministic derived chunk data and document persistence as a future concern.
- **[Risk] Overlay metrics may still be misread without precise labels** → Mitigation: use explicit “retained” vs “rendered” wording in snapshot fields and overlay text.

## Migration Plan

1. Update OpenSpec deltas for chunk caching, debug snapshots, and overlay diagnostics.
2. Implement bounded retention in `World` and wire retention-window updates through `OverworldRuntime`.
3. Update overlay text and debug snapshot fields to match the new diagnostics.
4. Add regression tests for eviction, deterministic reload, and bounded debug metrics.
5. Validate that exploration causes counts to stabilize instead of growing without bound.

Rollback is straightforward: remove pruning and revert the overlay/debug snapshot fields to the previous unbounded-retention behavior.

## Open Questions

None at this time.
