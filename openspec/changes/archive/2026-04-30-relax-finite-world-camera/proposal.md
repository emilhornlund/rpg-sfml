## Why

The current overworld runtime exposes finite-world assumptions through public APIs and camera behavior: `Camera::update()` depends on `World` so it can clamp the frame to `getWorldSize()`, and the view never shows space outside the generated map. That coupling makes the camera harder to evolve independently and keeps finite world extents visible at API boundaries even though the next step only needs freer camera motion and cleaner ownership.

## What Changes

- Remove camera clamping to world bounds so camera framing follows the focus position directly instead of pinning the view to the finite map edges.
- **BREAKING** decouple `Camera` from `World` by changing the camera update contract to depend only on focus position and viewport sizing.
- Relax finite-world public APIs so camera-facing code no longer needs world-size accessors to compute framing.
- Update overworld rendering requirements so showing space outside the generated world is valid when the camera moves beyond map edges, while world traversal still enumerates only generated terrain.
- Add focused spec coverage for unclamped camera behavior, rendering near and beyond world edges, and the narrowed world-facing API surface.

## Capabilities

### New Capabilities
<!-- None. -->

### Modified Capabilities
- `overworld-vertical-slice`: change camera requirements from world-clamped framing to focus-driven framing, and update overworld rendering behavior to allow visible space beyond generated terrain near map edges

## Impact

- Affected code: `include/main/Camera.hpp`, `src/main/Camera.cpp`, `include/main/World.hpp`, `src/main/World.cpp`, `src/main/Game.cpp`, and overworld-focused tests under `tests/`
- Affected APIs: `Camera::update(...)` becomes world-independent, and finite-world accessors currently used for camera framing may be removed or de-emphasized
- No new dependencies or executable entry point changes
