## Context

The current overworld runtime already keeps terrain in chunk-addressed storage inside `World`, and tile classification is resolved by chunk key plus chunk-local coordinates. Despite that boundary, `Game::render()` still loops over every tile in the configured world each frame and performs a tile lookup for each coordinate before drawing. The active camera already provides a clamped `ViewFrame`, so the runtime has the data needed to restrict rendering to the visible region without changing the camera or terrain-generation model.

This change is cross-cutting across `Game`, `World`, and render-focused tests, and it introduces a performance-oriented boundary that should remain compatible with the current chunk cache and future chunk-level optimizations.

## Goals / Non-Goals

**Goals:**
- Replace full-world frame rendering with camera-bounded terrain traversal.
- Keep visible-terrain traversal aligned with `World`'s retained chunk data rather than reintroducing a flat render grid in `Game`.
- Preserve current visual output semantics: tile coloring still comes from tile type, and player rendering still uses the active camera framing.
- Define testable behavior for overscan, edge clipping, and chunk reuse during rendering queries.

**Non-Goals:**
- Introduce streaming or lazy chunk generation beyond the current retained world cache.
- Change camera motion, player movement, or terrain generation rules.
- Add SFML-specific batching primitives such as vertex arrays or texture atlases in this change.
- Redesign world storage away from chunk-addressed retained data.

## Decisions

### Decision: Put visible-terrain query logic behind `World`
- `Game` should stop deriving render bounds directly from world dimensions and should instead ask `World` for terrain intersecting the active `ViewFrame`.
- This keeps render-facing terrain traversal at the same boundary that already owns chunk lookup and chunk retention.
- Alternative considered: computing visible tile min/max directly in `Game`. This is smaller, but it duplicates knowledge about tile/world conversion and bypasses the existing chunk abstraction.

### Decision: Traverse chunk bounds first, then tiles within each visible chunk
- The visible query should derive tile bounds from the camera frame, convert those tile bounds into chunk bounds, and traverse only chunks intersecting that region.
- Within each chunk, the world should enumerate local tiles in a stable order and skip tiles outside world bounds or outside the visible tile rectangle.
- Alternative considered: direct tile-rectangle iteration without chunk awareness. That reduces API design work, but it leaves rendering disconnected from the chunk storage that already exists and makes later chunk-level batching harder.

### Decision: Include bounded overscan around the visible frame
- The visible query should include a small overscan margin around the computed visible tile rectangle so render results remain stable near frame edges and do not pop from float-to-tile rounding.
- Overscan must remain bounded and clipped to world bounds.
- Alternative considered: exact frame-to-tile clipping only. This is simpler but more sensitive to rounding behavior at the frame edge.

### Decision: Reuse retained chunk data for rendering queries
- Visible render traversal must read from the existing retained chunk cache and must not trigger a separate render-specific generation path.
- This preserves the current caching contract and lets render queries benefit from the same deterministic chunk data as ordinary tile lookups.
- Alternative considered: generating render slices independently. This would duplicate generation logic and risk divergence between gameplay and rendering queries.

### Decision: Cover the change with behavior-focused tests
- Specs and implementation should be backed by tests that verify camera-bounded traversal, clipping at world edges, and that repeated visible-render queries do not require additional chunk generation.
- Alternative considered: relying only on the existing render loop tests. That would leave the new query boundary underspecified.

## Risks / Trade-offs

- **Visible query API leaks render concerns into `World`** → Keep the API centered on world-facing concepts (`ViewFrame`, tile coordinates, chunk-backed traversal) instead of SFML types or draw calls.
- **Overscan can draw more than strictly necessary** → Keep overscan intentionally small and bounded so it smooths edge behavior without restoring near-full-world work.
- **Chunk-first traversal adds iteration complexity at world edges** → Explicitly clip both chunk traversal and per-tile emission to valid world bounds and cover partial edge chunks in tests.
- **Future batching needs could outgrow the first API shape** → Prefer a traversal/query boundary that exposes visible chunk/tile data without committing to a specific draw container implementation.

## Migration Plan

1. Add spec coverage for visible chunk rendering, overworld render behavior, and chunk-cache reuse during render traversal.
2. Introduce the world-facing visible-terrain query and update `Game::render()` to consume it.
3. Validate that player rendering and camera behavior stay unchanged while terrain rendering shifts to visible traversal.

No deployment or rollback steps beyond reverting the runtime and spec changes are expected for this local executable.

## Open Questions

- Whether overscan should be expressed in tiles or chunks can stay an implementation choice as long as the externally visible behavior remains bounded and edge-safe.
