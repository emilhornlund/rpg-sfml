## Context

The current overworld view uses the window size directly as the gameplay camera frame, which makes a 16x16 tile world appear too small during development. The runtime already has a clean separation between the SFML shell (`Game`) and gameplay-facing coordination (`OverworldRuntime`, `Camera`, `Player`, `World`), so the change should preserve those boundaries while adding debug-only controls for zoom and a grid overlay.

This change crosses multiple modules. Camera framing influences visible tile selection through `World::getVisibleTiles`, movement speed lives in `Player`, player animation timing also lives in `Player`, and debug hotkeys plus render-only overlay drawing live in `Game`. The implementation therefore needs an explicit design so the global default zoom, optional debug controls, and player pacing remain coherent.

## Goals / Non-Goals

**Goals:**
- Add debug-only hotkeys for zooming the overworld view in 50% steps.
- Start all modes at a 300% zoom level so the default view is meaningfully tighter than the current default.
- Keep zoom ownership in gameplay-facing camera/runtime state so visible-tile culling stays aligned with the rendered frame.
- Add a debug-only tile-grid overlay drawn above terrain and below the player.
- Scale player movement speed with debug zoom so traversal keeps a roughly stable on-screen pace.
- Slow the baseline player movement and walk animation cadence so default traversal reads like walking rather than sprinting.

**Non-Goals:**
- Adding user-facing settings menus or persisted debug preferences.
- Changing normal gameplay behavior outside development/test mode.
- Introducing new rendering layers or a broader debug UI system.
- Reworking the world-generation or camera-follow architecture beyond what debug zoom requires.

## Decisions

### Keep debug input at the shell boundary

Debug hotkeys will be polled and interpreted in `Game::processEvents()`, alongside the existing directional input handling. The SFML shell will translate those key presses into repo-native debug control state before passing the resulting frame inputs into `OverworldRuntime`.

**Why:** this preserves the existing input-boundary rule that backend-specific key inspection stays at the shell edge.

**Alternatives considered:**
- Inspect SFML key codes inside `OverworldRuntime` or `Camera`: rejected because it breaks the gameplay/runtime boundary.
- Handle zoom only inside rendering code: rejected because `World::getVisibleTiles()` depends on the camera frame, so render-only zoom would desynchronize culling from the displayed view.

### Own zoom in gameplay-facing camera/runtime state

The active debug zoom level will be represented as gameplay-facing runtime state and applied when computing the camera frame size. `Game::render()` will continue constructing the SFML `View` from the published camera frame rather than owning separate zoom math.

**Why:** the camera frame is already the contract used for terrain visibility and rendering, so zoom must affect that shared contract.

**Alternatives considered:**
- Use `sf::View::zoom()` as the primary source of truth: rejected because visible terrain enumeration would still use the unzoomed frame.
- Store zoom only in `Game`: rejected because movement scaling and camera frame publication would then need shell-owned gameplay logic.

### Keep the grid overlay render-only

Grid visibility will be stored as debug state and consumed in `Game::render()`, where the overlay can be drawn after terrain tiles and before player markers using the already-published visible tile geometry.

**Why:** the overlay is purely presentation and does not need to expand gameplay-owned render snapshot types.

**Alternatives considered:**
- Publish explicit grid primitives in the render snapshot: rejected as unnecessary growth in the gameplay/render contract for a debug-only presentation aid.

### Use 300% as the global default zoom

The camera module will default to 300% zoom for every runtime mode, not only development/test mode. Debug input may still adjust zoom during development and testing, but the baseline framing will remain the same everywhere.

**Why:** the 100% window-sized view is too far out to be playable on a 16x16 tile world.

**Alternatives considered:**
- Keep 300% only for debug mode: rejected because the normal default view remains far too zoomed out to play comfortably.

### Scale movement speed relative to the default debug zoom

Movement speed will be derived from the active debug zoom relative to the default 300% zoom when debug zoom controls are active. Zooming out increases world-space speed; zooming in decreases it.

**Why:** this keeps traversal feeling roughly stable on screen as the visible world area changes during debugging without forcing non-debug play to depend on debug state.

**Alternatives considered:**
- Keep world movement speed fixed while debugging: rejected because zoomed-in movement would feel too fast on screen while zoomed-out traversal would feel sluggish for debugging.

### Retune baseline player pacing with time-based animation

Player movement and walking animation will keep using time-based cadence, but both baseline constants will be reduced from the previous fast values.

**Why:** the original time-based animation feel was better than the later step-progress experiment, and the tighter default zoom makes the previous speed values feel too aggressive.

**Alternatives considered:**
- Keep the existing faster baseline movement and animation values: rejected because the player reads more like a sprinting vehicle than a walking character at the new default zoom.

## Risks / Trade-offs

- **[Debug-only mode can leak into normal gameplay]** → Keep hotkeys and grid overlay gated behind explicit development/test-mode state while allowing the baseline 300% zoom to apply everywhere.
- **[Zoom ranges can produce awkward visibility or overly fast traversal]** → Clamp zoom to a fixed ladder and tie movement scaling to the chosen default zoom level.
- **[Animation frame transitions may look abrupt at step boundaries]** → Use step-progress thresholds that preserve the current three-frame walking pattern and settle to the idle frame when the player is not moving.
- **[Rendering the grid overlay could add clutter]** → Keep it toggleable and render it with lightweight line geometry over only the visible tiles.

## Migration Plan

1. Add debug control state and input translation for zoom and grid toggling.
2. Extend overworld runtime/camera flow to publish camera frames using the 300% global default zoom and any active debug zoom override.
3. Apply zoom-relative movement-speed logic for debug zoom overrides and retune the baseline movement/animation constants in `Player`.
4. Draw the optional grid overlay in the render phase between terrain and player rendering.
5. Update focused tests for input translation, runtime camera framing, movement scaling, and animation progression.

Rollback is straightforward: remove the debug control path and restore fixed viewport-sized framing and time-based walk animation.

## Open Questions

None. The current direction is: 300% default zoom for all modes, 50% debug zoom steps, debug-only grid overlay, zoom-relative movement scaling while debugging, and slower baseline walk pacing.
