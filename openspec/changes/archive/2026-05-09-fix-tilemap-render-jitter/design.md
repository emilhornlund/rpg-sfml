## Context

The current overworld loop updates gameplay with the elapsed wall-clock delta for each frame and immediately renders the resulting snapshot. That keeps the shell simple, but it couples simulation pacing to display pacing and publishes camera centers that can land between pixels while the player moves in fractional world-space increments.

The visible symptom is tilemap shimmer that becomes easier to notice when zoomed out. The project already disables texture smoothing, so the remaining instability is caused by camera presentation rather than texture sampling configuration. A fixed-step accumulator experiment made the player feel juddery and degraded the walk animation cadence, so the retained solution keeps the smoother frame-variable movement path and narrows the change to render-facing camera behavior and explicit pacing configuration.

Constraints:
- Keep `Game` as the SFML-facing shell and preserve the existing `OverworldRuntime` boundary.
- Stay within the current SFML/CMake setup with no new runtime dependencies.
- Preserve repo-native render snapshot types rather than pushing SFML types into gameplay modules.

## Goals / Non-Goals

**Goals:**
- Preserve the existing responsive frame-variable movement and player animation behavior.
- Publish render-facing camera and marker data that can be presented without tilemap shimmer during movement.
- Make the render camera snap at the current zoom's screen-pixel granularity instead of coarse whole-world-unit increments.
- Keep the solution narrow and aligned with the current `Game` / `OverworldRuntime` / `Camera` boundaries.

**Non-Goals:**
- Reworking overworld movement rules, collision rules, or terrain generation.
- Introducing a new rendering backend, scene graph, or ECS layer.
- Changing art assets, tile geometry baseline, or tileset metadata formats.
- Replacing the current frame-variable movement loop with a fixed-timestep simulation architecture.
- Adding interpolation machinery for render snapshots or gameplay entities.

## Decisions

### Preserve the existing frame-variable gameplay loop

`Game` should keep the current event -> update -> render frame lifecycle driven by the current frame delta. That preserves the smoother player motion and walk animation cadence the project already had, and keeps the change focused on camera presentation instead of widening it into a timing architecture rewrite.

This keeps SFML concerns in the shell, preserves the current architecture, and avoids reintroducing the judder observed when render frames were produced without a fresh simulation update.

**Alternatives considered**
- Replace the loop with a fixed-timestep accumulator: rejected because the current gameplay and presentation layers do not yet have the interpolation support needed to avoid visible judder and animation regressions.
- Move timing changes into `OverworldRuntime`: rejected because it would still need the same broader interpolation work while also weakening the current shell/runtime boundary.

### Make camera presentation pixel-stable at the current zoom

The camera should retain gameplay-facing world-space tracking, but the frame used for terrain rendering should be snapped before it is passed to SFML view setup. The snap step should be derived from the current camera frame size and the active viewport pixel dimensions, so one screen pixel maps to one snap increment in world space at the current zoom.

This directly targets the observed shimmer because terrain tiles are positioned on a 16-unit grid and currently move under a fractional camera center. Using a zoom-aware snap step avoids the coarse "camera lag" that whole-world-unit snapping introduced.

**Alternatives considered**
- Snap every visible tile and object independently at draw time: more invasive and easier to apply inconsistently across render paths.
- Snap the camera to whole world units: stable, but visibly too coarse when the zoom level makes one screen pixel smaller than one world unit.
- Leave the camera unsnapped: keeps the player tightly centered, but preserves the tile shimmer the change is meant to address.

### Use one camera frame for both culling and drawing

The same snapped render-facing camera frame should be used both to collect visible terrain/content and to configure the SFML view for the frame. That avoids tile-edge flashing caused by culling or autotile selection being computed from one camera frame while drawing uses another.

This keeps the runtime coherent: one presentation camera, one visible set, one draw view.

**Alternatives considered**
- Build visibility from the gameplay camera and then draw with a separately snapped camera: rejected because it can produce border flashing and inconsistent visible tile sets.
- Introduce previous/current render snapshots and interpolate between them: rejected for now because it expands the change beyond the narrow camera-presentation fix that is actually being kept.

### Make frame pacing configuration explicit

The runtime should expose one active display pacing mode at a time. If vertical sync is enabled, the framerate limiter should not also be enabled. If vertical sync is disabled, an explicit limiter may be used as a fallback.

This removes ambiguous pacing behavior and aligns the change with common SFML guidance.

**Alternatives considered**
- Keep the current fixed framerate limit only: acceptable, but it leaves tearing/pacing behavior less adaptable across systems.
- Enable both VSync and limiter together: easy to configure, but can create inconsistent frame pacing and hide the real source of jitter.

## Risks / Trade-offs

- **Accumulator catch-up can cause bursts of updates after a long frame** → Clamp incoming frame time and keep render to one pass per frame.
- **Zoom-aware snapping still quantizes camera motion** → Use viewport-derived pixel increments rather than whole-world-unit rounding to keep the effect visually subtle.
- **Incorrect viewport data would produce the wrong snap step** → Retain the active viewport size in the runtime and derive snap increments from the published camera frame and current viewport dimensions together.
- **Pixel snapping can make non-tile-aligned entities feel less smooth if applied too broadly** → Snap only the render-facing camera frame used for terrain presentation and keep gameplay-facing player movement unsnapped.
- **VSync behavior varies by platform and driver** → Keep the pacing mode explicit and test with the repo’s existing runtime paths rather than assuming one mode is universally best.

## Migration Plan

No data migration is required. The change is runtime-only and can be shipped behind the existing executable entry point without altering saved data, asset formats, or build layout.

Rollback is straightforward: restore the unsnapped render camera presentation if the zoom-aware snapping still produces undesirable feel on target systems.

## Open Questions

- Does the zoom-aware camera snapping fully address the original shimmer on the most problematic zoom levels, or is a narrower follow-up change still needed for specific autotile edges?
