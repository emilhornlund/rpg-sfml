## 1. Debug input and runtime control plumbing

- [x] 1.1 Add debug-only overworld view control state and hotkey translation at the `Game` shell boundary for zoom in, zoom out, and tile-grid toggle actions.
- [x] 1.2 Extend repo-native overworld frame input/runtime state so debug zoom and overlay settings can be consumed without exposing SFML input types beyond the shell boundary.

## 2. Camera framing and traversal behavior

- [x] 2.1 Update overworld runtime and camera state so all modes start at 300% zoom and development/test mode applies 50% zoom increments to the published camera frame.
- [x] 2.2 Scale player movement speed from the active debug zoom relative to the default debug zoom while preserving tile-centered cardinal step completion behavior.
- [x] 2.3 Retune baseline player movement and walking animation cadence while preserving the time-based animation model.

## 3. Debug rendering and test coverage

- [x] 3.1 Render the optional tile-grid overlay above terrain and below the player using visible tile geometry from the current frame.
- [x] 3.2 Add or update focused tests for debug input translation, zoom-adjusted camera framing, zoom-relative movement speed, and slower time-based animation behavior.
- [x] 3.3 Add or update focused tests for tile-grid overlay toggling and debug-only gating so normal gameplay behavior remains unchanged outside development/test mode.
