## Why

Overworld movement currently renders with visible tilemap shimmer, especially while moving and when zoomed out. The runtime advances simulation with frame-variable delta time and renders against fractional camera positions, which makes terrain appear to flicker or lag even though texture smoothing is already disabled.

## What Changes

- Add a stable overworld rendering capability that defines frame pacing and presentation requirements for the overworld game loop.
- Keep the existing frame-variable overworld movement path so player motion and walk animation cadence remain responsive.
- Require pixel-aligned camera presentation for terrain-facing rendering so tilemaps do not shimmer during player movement.
- Snap the render-facing camera to a zoom-aware viewport pixel grid instead of coarse whole-world-unit rounding so the camera stays visually stable without obviously lagging behind the player.
- Clarify runtime frame-pacing configuration so the game shell uses either vertical sync or a frame limiter, but not both together.

## Capabilities

### New Capabilities
- `stable-overworld-rendering`: Defines stable overworld presentation through explicit frame pacing, consistent render-camera snapping, and zoom-aware pixel alignment.

### Modified Capabilities

## Impact

- Affected code: `src/main/Game.cpp`, `src/main/OverworldRuntime.cpp`, `src/main/Camera.cpp`, related runtime support helpers, and focused runtime/render tests.
- Affected systems: overworld camera framing, render snapshot publication, and SFML window pacing configuration.
- Dependencies: no new external dependencies expected; work stays within the current SFML/CMake runtime structure.
