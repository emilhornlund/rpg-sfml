## Context

The current generated-content/player occlusion path renders correctly but scales poorly with fullscreen resolutions and zoomed-out views. The shell currently collects every generated-content entry that sorts in front of the player, draws the player into one full-window render texture, draws all front occluders into a second full-window render texture, and composites the result through a fullscreen shader.

Recent measurement on this repository showed the largest fullscreen regression comes from the occlusion path itself rather than from terrain batching. At `3840x1600 @ 300%`, disabling the occlusion silhouette path improved frame rate from roughly `27-29 FPS` to `41-44 FPS`, while disabling vegetation alone improved only to `32-33 FPS`. The current implementation also uses a broad front-occluder count that does not distinguish between entries that sort in front of the player and entries that actually overlap the player's presentation bounds on screen.

This change needs to reduce the occlusion path cost while preserving the current player-readability behavior: the normal player sprite stays visible where unobstructed, the silhouette appears only where front generated content hides the player, and resize-sensitive screen-space composition remains aligned with the player.

## Goals / Non-Goals

**Goals:**
- Qualify occlusion work from actual overlap with the player presentation area instead of all front occluders in y-sorted order.
- Skip the occlusion pass entirely when no overlap-qualified occlusion candidates exist for the frame.
- Reduce the pixel cost of the occlusion masks by allowing a lower-resolution working surface while preserving the current silhouette-only output behavior.
- Publish diagnostics that distinguish front occluders from overlap-qualified occlusion candidates so performance work can be reasoned about from the debug overlay.
- Preserve current gameplay/module boundaries so `World`, `Player`, and `OverworldRuntime` remain SFML-agnostic.

**Non-Goals:**
- Redesign the underlying y-sorted object ordering rules.
- Batch or otherwise redesign the main vegetation rendering pass.
- Implement the more invasive player-local mask-region approach in this first change.
- Change asset formats, generated-content metadata, or gameplay-visible collision/visibility rules.

## Decisions

### Decision: Qualify occlusion candidates by player-overlap bounds

The shell will keep the current deterministic y-sorted render queue, but it will narrow the occlusion list from "all generated-content entries after the player marker" to "front generated-content entries whose published world-space bounds intersect the player's published world-space presentation bounds."

**Why this over keeping the current broad front-occluder set?**
- The current front-occluder count is a sort-order metric, not an actual overlap metric, so it overstates the work required for the silhouette path.
- The published render snapshot already contains enough world-space geometry to evaluate overlap without pushing SFML concerns into gameplay-facing modules.

**Alternative considered:** move overlap qualification into `OverworldRuntime`. Rejected because occlusion remains a render-shell concern and should stay below the gameplay/render boundary.

### Decision: Skip the silhouette pass when no overlap-qualified candidates exist

The renderer will bypass player-mask rendering, occluder-mask rendering, and the composite draw when the overlap-qualified occlusion candidate list is empty for the current frame.

**Why this over always running the pass when any front occluder exists?**
- It removes the full mask/composite cost on frames where sort order alone does not produce actual player overlap.
- It keeps the visible output unchanged, because a frame without overlap-qualified occlusion does not need a silhouette result.

**Alternative considered:** keep the current broad trigger and optimize only the mask resolution. Rejected because skipping unnecessary passes is cheaper and lower risk than paying any mask cost on empty-overlap frames.

### Decision: Render occlusion masks through a reduced-resolution working surface

The player mask and occluder mask will be rendered through a configurable reduced-resolution surface derived from the active window size, then sampled back during composite. The first implementation should target a simple full-frame downsampled mask rather than a player-local sub-rectangle.

**Why this over jumping directly to a player-local mask region?**
- A downsampled full-frame path directly attacks the resolution-sensitive hotspot with a smaller change to the current architecture.
- It preserves the existing screen-space composite flow and resize-handling model, which makes the first implementation easier to validate.

**Alternative considered:** render only a player-local mask region. Deferred because it needs more coordinate plumbing, tighter edge handling, and a larger behavior-validation surface.

### Decision: Publish both broad and overlap-qualified occlusion diagnostics

The debug overlay will continue to report the broad front-occluder count and will add a separate overlap-qualified occlusion candidate count for the current frame.

**Why this over replacing the old metric?**
- The broad front-occluder count is still useful for reasoning about sort-order composition.
- Showing both metrics makes it obvious how much work is removed by overlap culling and keeps performance investigations grounded in frame data.

**Alternative considered:** expose only the overlap-qualified count. Rejected because it would remove a useful diagnostic for verifying render-queue ordering behavior.

### Decision: Defer cheaper occluder-mask inputs to follow-up work

This change will continue to build the occluder mask from existing generated-content rendering data after overlap culling, while documenting cheaper proxy geometry or batched mask inputs as a follow-up option if the reduced-resolution pass is still too expensive.

**Why this over changing occluder-mask drawing immediately?**
- The measurements show the dominant regression is the occlusion pass itself, especially its fullscreen working surfaces, rather than only the vegetation draw path inside that pass.
- Deferring proxy-mask rendering keeps the first change focused and easier to validate.

**Alternative considered:** replace occluder-mask draws with simplified rectangles or batched proxy geometry in the same change. Deferred because it changes visual fidelity risk at the same time as the mask-resolution change.

## Risks / Trade-offs

- **[Lower-resolution masks may soften silhouette edges]** → Mitigation: keep the output constrained to the current silhouette color/effect and choose a conservative starting scale such as half resolution.
- **[World-space overlap tests may miss edge-touching occluders if bounds math drifts from render placement]** → Mitigation: derive overlap bounds from the same published size/origin/position data already used for drawing.
- **[Broad and overlap-qualified counts could diverge in confusing ways]** → Mitigation: document each metric's meaning in tests and keep both counts visible in the debug overlay.
- **[Fullscreen downsampling may still leave too much work on very large displays]** → Mitigation: document the player-local mask-region path as the next optimization step if the first implementation is insufficient.

## Migration Plan

1. Add render-side overlap qualification helpers and diagnostics for overlap-qualified occlusion candidates.
2. Switch the occlusion trigger from broad front-occluder existence to overlap-qualified candidate existence.
3. Introduce a reduced-resolution occlusion working surface while preserving the current composite behavior.
4. Update rendering and debug-overlay tests to validate overlap qualification, diagnostics, and resize-safe silhouette alignment.
5. If regressions appear, rollback is a straightforward reversion to the prior full-resolution broad-trigger path because no gameplay or asset contracts change.

## Open Questions

- What downsample factor gives the best trade-off between performance and acceptable silhouette edge quality in this project's art style?
- Should the reduced-resolution occlusion scale be fixed for now or surfaced as a small render-side constant/configuration point?
- If the first pass is still too expensive on large displays, should the next follow-up prefer player-local masks or cheaper occluder-mask proxy geometry first?

## Follow-up Note

If post-change profiling still shows a meaningful fullscreen regression, the next optimization should prioritize a player-local occlusion mask region before cheaper occluder-mask proxy geometry. The existing investigation showed the dominant cost came from the full-window occlusion working surfaces more than from vegetation drawing alone, so localizing the mask region is the most direct next step if half-resolution masks are not enough.
