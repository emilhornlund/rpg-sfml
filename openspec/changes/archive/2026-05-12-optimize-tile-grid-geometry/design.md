## Context

The current debug tile-grid overlay is already submitted as a single batched vertex array, but the builder still expands each visible tile into four edge rectangles. Because the visible tile set comes from a rectangular camera query, this approach emits `4 * W * H` rectangles for a visible area of `W` columns by `H` rows even though only `W + 1` vertical boundaries and `H + 1` horizontal boundaries exist.

The optimization is not purely a geometry-count rewrite. The current per-tile overlay helper places top and left edges at each tile origin and bottom and right edges at `size - thickness`, so interior boundaries become two adjacent strips rather than one overlapping strip. That means interior grid lines currently render at twice the configured line thickness while the outer border renders at the configured thickness. The new builder needs to preserve that appearance while reducing rectangle count.

## Goals / Non-Goals

**Goals:**
- Rebuild tile-grid geometry from the visible tile bounds instead of per-tile edge expansion.
- Reduce rectangle count from `4 * W * H` to `(W + 1) + (H + 1)` while preserving current world-space alignment.
- Preserve the current visual thickness semantics: outer borders remain one line-thickness wide and interior boundaries remain visually equivalent to the existing doubled-strip result.
- Keep the optimization entirely in render-side helpers without changing render snapshots, world visibility collection, or debug controls.

**Non-Goals:**
- Changing terrain batching, vegetation rendering, or any non-grid draw path.
- Modifying `World::getVisibleTiles()` or adding new render-snapshot fields.
- Redefining the tile-grid overlay to use one-pixel line primitives or a different visual style.

## Decisions

### Decision: Derive one grid rectangle per visible row and column boundary

The new grid builder will scan `renderSnapshot.visibleTiles` to determine the minimum and maximum visible tile coordinates and world-space extents, then emit long horizontal and vertical filled rectangles for those boundaries.

**Why this over continuing to reuse the per-tile edge helper?**
- The visible tile query is rectangular, so the bounds fully describe the grid that needs to be shown.
- Emitting one strip per boundary removes duplicated interior-edge geometry and scales linearly with visible width and height instead of visible tile count.

**Alternative considered:** keep the current per-tile builder and attempt to cull duplicated edges. Rejected because it still requires tile-by-tile edge emission logic and does not simplify the geometry model as cleanly as bounds-derived strips.

### Decision: Preserve current thickness semantics explicitly

The strip builder will keep outer boundaries at the configured `lineThickness` and emit interior boundaries so they remain visually equivalent to the current two-adjacent-strip result.

**Why this over switching every boundary to a single `lineThickness` strip?**
- A single-thickness interior strip would visibly thin all interior grid lines compared with the current overlay.
- Preserving the existing appearance keeps the optimization behavior-safe and avoids changing the debug view while improving geometry efficiency.

**Alternative considered:** accept the visual change in exchange for even simpler geometry. Rejected because the current change is intended as a performance optimization, not a debug-visual redesign.

### Decision: Treat the visible tile set as a rectangular contract and validate assumptions in tests

The builder will rely on the current `World::getVisibleTiles()` behavior that returns every tile inside the visible rectangular bounds, and tests will cover representative multi-tile cases to ensure the generated geometry matches those bounds-derived expectations.

**Why this over introducing new explicit bounds in the render snapshot?**
- The snapshot already contains enough information to derive the grid bounds.
- Reusing existing snapshot data keeps the gameplay/render boundary unchanged.

**Alternative considered:** publish visible bounds directly from gameplay. Rejected because it would widen the snapshot contract for a render-shell optimization that can already be derived from existing data.

## Risks / Trade-offs

- **[The rectangular visible-tile assumption could drift later]** → Mitigation: document the assumption in the design/specs and cover multi-tile rectangular cases in tests so regressions surface quickly.
- **[The optimized strips could shift by a pixel or alter line widths]** → Mitigation: define geometry from the same world-space tile extents already used today and preserve current outer/interior thickness semantics explicitly.
- **[Debug overlay metrics will drop sharply after the optimization]** → Mitigation: treat the lower vertex count as an expected outcome and update tests to assert the new batched geometry scale.

## Migration Plan

1. Replace the per-tile grid batch builder with a bounds-derived strip builder in the render-side helper.
2. Update rendering-focused tests to assert the new rectangle and vertex counts for multi-tile visible areas.
3. Keep the existing render call site and debug toggle behavior unchanged so rollback remains a direct reversion of the helper and its tests.

## Open Questions

- No blocking product questions remain. The main coding-time detail is whether to assert or silently assume the rectangular visible-tile contract inside the helper.
