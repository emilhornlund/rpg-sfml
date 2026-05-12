## Why

Fullscreen and zoomed-out overworld rendering currently loses a large amount of frame rate even after terrain and grid batching. Recent investigation showed the dominant regression comes from the player occlusion silhouette path, which still performs full-window mask rendering whenever any front occluder exists, even if most front occluders do not overlap the player on screen.

## What Changes

- Tighten player-occlusion candidate selection so the silhouette path considers only front generated-content entries that actually intersect the player presentation area instead of every front occluder in y-sorted order.
- Allow the shell to skip the occlusion mask/composite pass entirely when no front generated-content entry overlaps the player for the current frame.
- Allow the silhouette mask pass to render through a lower-resolution working surface while preserving the current visual requirement that only the occluded portion of the player is revealed as a silhouette.
- Publish render-side diagnostics that distinguish broad front-occluder counts from overlap-qualified occlusion candidates so performance work can be reasoned about from the debug overlay.
- Document a follow-up path toward a player-local occlusion region and cheaper occluder-mask inputs without requiring that more invasive design in the first implementation.

## Capabilities

### New Capabilities
- `player-occlusion-optimization`: Defines overlap-qualified occlusion candidates and lower-cost silhouette composition behavior for generated-content/player overlap.

### Modified Capabilities
- `generated-content-rendering`: The occlusion silhouette requirements will change so the shell qualifies front occluders by actual player overlap and may use a lower-resolution mask/composite path while preserving silhouette-only output.
- `debug-overlay-display`: The debug overlay requirements will change so render diagnostics can expose overlap-qualified occlusion candidate counts alongside existing render metrics.

## Impact

- Affected code: `src/main/Game.cpp`, occlusion and render helpers under `src/main/`, runtime debug snapshot plumbing, and rendering-focused tests in `tests/`.
- Affected systems: generated-content rendering, player occlusion silhouette composition, debug overlay metrics, and fullscreen/zoom stress behavior.
- No intended gameplay API changes for `World`, `Player`, `Camera`, or asset formats.
