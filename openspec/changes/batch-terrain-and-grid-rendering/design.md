## Context

The current overworld renderer submits terrain and debug tile-grid primitives one item at a time from `Game::render()`. Terrain currently performs one textured sprite draw per visible tile, and the debug tile grid performs four rectangle draws per visible tile segment. Recent profiling on this repository showed that normal 1080p fullscreen and moderate zoom changes remain within the current 60 FPS pacing budget, while the debug tile grid causes a large frame-time regression because it multiplies visible-tile work into thousands of small SFML draw calls.

This change needs to reduce render-shell submission overhead without changing the gameplay-facing snapshot boundary. `World` and `OverworldRuntime` must remain SFML-agnostic, terrain selection must remain driven by the existing tileset metadata and autotile logic, and the pixel-stable camera behavior must remain intact across zoom and resize changes.

## Goals / Non-Goals

**Goals:**
- Replace per-tile terrain sprite submission with batched textured geometry owned by the render shell.
- Replace per-segment tile-grid rectangle submission with batched colored geometry owned by the render shell.
- Preserve current terrain visuals, autotile determinism, zoom behavior, and debug tile-grid semantics.
- Keep batching as an implementation detail below the gameplay/render snapshot boundary.

**Non-Goals:**
- Batch vegetation, player markers, or the player occlusion silhouette pass.
- Redesign world visibility collection, chunk retention, or render snapshot contents.
- Introduce new asset formats, new runtime dependencies, or gameplay-facing SFML types.

## Decisions

### Decision: Add render-side batch builders for terrain and grid geometry

The render shell will build dedicated vertex-array-backed geometry for the visible terrain layer and the optional tile-grid overlay layer. This keeps batching close to the existing SFML drawing code and avoids leaking SFML vertex concerns into gameplay modules.

**Why this over changing render snapshots?**
- The current snapshot already provides the world-space tile geometry and tile identity needed to build terrain and grid quads.
- Keeping snapshots unchanged preserves existing module boundaries and avoids ripple effects into gameplay tests.

**Alternative considered:** publish precomputed atlas coordinates or vertex data from `OverworldRuntime`. Rejected because it would push render-backend concerns into gameplay-facing code.

### Decision: Preserve current autotile selection and generate terrain quads from the existing selection path

Terrain batching will still derive atlas cells from the current metadata-backed terrain selection path. The batch builder will translate each visible tile into four textured vertices using the selected atlas rect, rather than issuing a sprite draw for that tile.

**Why this over chunk-cached static meshes?**
- It preserves correctness for animated water tiles and current zoom/camera behavior with a smaller first step.
- It reuses the existing autotile rules instead of introducing a second terrain selection path.

**Alternative considered:** cache chunk meshes across frames. Deferred because water animation and view-dependent visibility make the invalidation story more complex than needed for the first change.

### Decision: Batch the debug tile grid as filled quads, not line primitives

The grid overlay will be emitted as colored quad geometry that preserves the current line thickness and rectangle-based appearance. Using filled quads matches the existing visual model more closely than line primitives, which may have different thickness or rasterization behavior across drivers.

**Why this over `sf::Lines` or keeping rectangles?**
- Quads preserve the current explicit line thickness behavior.
- Rectangles would keep the draw-count problem unchanged.

**Alternative considered:** use `sf::Lines`. Rejected for this change because the current grid is specified by rectangle thickness rather than one-pixel line primitives.

### Decision: Rebuild batches from the current snapshot each rendered frame

The renderer will rebuild terrain and grid batches from the active render snapshot each frame instead of introducing incremental mesh mutation or cross-frame cache invalidation.

**Why this over persistent cached geometry?**
- It keeps the implementation straightforward and easier to validate.
- It already removes the most obvious bottleneck: thousands of small SFML draw submissions.

**Alternative considered:** only rebuild when the visible tile set changes. Deferred because animated terrain and view changes would still require careful invalidation logic.

## Risks / Trade-offs

- **[Batch build cost replaces draw-call cost]** → Mitigation: keep builders linear over visible tiles, reuse buffers where practical, and validate behavior under resize and zoom stress cases.
- **[Terrain batching could accidentally diverge from current autotile selection]** → Mitigation: route atlas selection through the existing terrain metadata helpers and keep rendering-boundary tests focused on the same texture-selection path.
- **[Grid visuals could shift by a pixel if quad generation differs from rectangle placement]** → Mitigation: derive quad geometry from the same overlay rectangle helper already used by the current grid path.
- **[This change leaves vegetation and occlusion costs untouched]** → Mitigation: scope the work explicitly to the confirmed terrain/grid draw-call hotspot and treat vegetation batching as a follow-up if needed.

## Migration Plan

1. Introduce render-side helpers for terrain-batch and grid-batch construction.
2. Switch `Game::render()` from per-tile/per-rectangle submission to batched submission while preserving the existing render order.
3. Update rendering-boundary tests to validate the new batching path.
4. If regressions appear, rollback is a straightforward reversion to the prior per-item draw path because no gameplay or asset contracts change.

## Open Questions

- No blocking product questions remain for the proposal. The main implementation choice left to coding time is the exact helper/file split for the new batch builders.
