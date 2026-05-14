## 1. Debug overlay extraction

- [x] 1.1 Extract debug overlay frame state, layout, and draw orchestration out of `src/main/Game.cpp`
- [x] 1.2 Move any remaining overlay-specific state ownership to a dedicated support collaborator without changing overlay behavior
- [x] 1.3 Add or update focused tests for the extracted debug overlay boundary and keep existing overlay behavior covered

## 2. Resource bootstrap and input translation

- [x] 2.1 Extract shell-owned resource bootstrap for textures, font, and shader loading out of `Game.cpp`
- [x] 2.2 Extract SFML event polling and key-to-runtime action translation out of `Game.cpp`
- [x] 2.3 Update runtime-shell tests so constructor/setup and event handling boundaries remain explicit after extraction

## 3. Render boundary preparation

- [x] 3.1 Replace `Game.cpp` literal rendering boundary checks with module-boundary assertions that tolerate helper-owned render logic
- [x] 3.2 Preserve coverage for the responsibilities that must remain in the top-level `Game` shell after boundary-test updates

## 4. Render-plan extraction

- [x] 4.1 Extract render-queue construction, stable ordering, and player-marker lookup out of `Game.cpp`
- [x] 4.2 Extract front-occluder qualification and debug render-metric preparation out of `Game.cpp`
- [x] 4.3 Add or update focused tests for the extracted render-plan boundary

## 5. Player occlusion compositor extraction

- [x] 5.1 Extract reduced-resolution occlusion surface sizing and render-texture preparation out of `Game.cpp`
- [x] 5.2 Extract occluder-mask drawing, shader uniform setup, and silhouette composite drawing into a dedicated support collaborator
- [x] 5.3 Add or update focused tests for the extracted occlusion compositor boundary while preserving current occlusion behavior

## 6. Overworld scene renderer extraction

- [x] 6.1 Extract terrain, generated-content, and tile-grid pass orchestration out of `Game.cpp`
- [x] 6.2 Keep `Game.cpp` as a high-level frame coordinator that wires together the extracted shell concerns
- [x] 6.3 Run and update the relevant runtime-shell tests so the final coordinator boundary stays explicit and behavior remains unchanged
