## 1. Runtime presentation updates

- [x] 1.1 Add explicit resize-aware screen-space presentation state to the game shell for shell-owned overlay rendering.
- [x] 1.2 Update window event/render flow so overlay composition uses the current window dimensions after resize instead of implicit default-view behavior.

## 2. Silhouette alignment behavior

- [x] 2.1 Keep the player occlusion mask and occluder mask generation on the world-space view while compositing the final silhouette through the explicit screen-space overlay path.
- [x] 2.2 Ensure the silhouette remains centered on the same on-screen position as the player sprite after window resizing.

## 3. Regression coverage

- [x] 3.1 Extend runtime-facing tests to cover resize-aware overlay presentation behavior.
- [x] 3.2 Add or update silhouette-focused checks so resized-window rendering keeps the occlusion silhouette aligned with the player.
