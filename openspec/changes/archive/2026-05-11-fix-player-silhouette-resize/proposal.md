## Why

Resizing the game window can shift the player occlusion silhouette away from the player sprite and eventually move it out of view. The overworld render path needs a resize-safe presentation contract so the silhouette stays centered on the player after the viewport changes.

## What Changes

- Update the overworld presentation requirements so resize handling keeps the occlusion silhouette aligned with the player sprite after window size changes.
- Define a consistent separation between world-space rendering and screen-space compositing for the silhouette overlay.
- Add implementation work to make the runtime own an explicit resize-aware screen-space view for overlay composition instead of relying on implicit default-view behavior.

## Capabilities

### New Capabilities
<!-- None. -->

### Modified Capabilities
- `generated-content-rendering`: tighten the occlusion-silhouette behavior so the overlay stays registered to the player when the window is resized.
- `game-runtime`: require explicit resize-aware screen-space presentation handling for shell-owned overlay composition.

## Impact

- Affected specs: `openspec/specs/generated-content-rendering/spec.md`, `openspec/specs/game-runtime/spec.md`
- Affected code: `src/main/Game.cpp` and related render/runtime support tests
- No public API or dependency changes
