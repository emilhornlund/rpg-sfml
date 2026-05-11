## Context

The overworld renderer currently draws terrain, vegetation, and the player in world space from the render snapshot's camera frame, then composites the player occlusion silhouette as a later pass. The resize bug appears only in that silhouette pass: after the window size changes, the silhouette can drift away from the player sprite even though the player and world continue to render correctly.

This points to a mismatch between the coordinate system used to build the silhouette masks and the coordinate system used to composite the final overlay. The current implementation relies on implicit default-view behavior for screen-space drawing, which is brittle when the window size changes mid-session.

## Goals / Non-Goals

**Goals:**
- Keep the player occlusion silhouette centered on the same on-screen position as the player sprite before and after window resizing.
- Make resize handling explicit for screen-space composition owned by the game shell.
- Preserve the current world-space render snapshot and shader-based silhouette approach.
- Keep the change local to the runtime shell and related tests.

**Non-Goals:**
- Redesign the overworld render order or replace the current y-sorted object presentation model.
- Change gameplay-owned camera, player, or world responsibilities.
- Replace the silhouette shader with a different occlusion effect.

## Decisions

### Use an explicit resize-aware screen-space view for overlay composition

The game shell will own a dedicated screen-space view whose size and center are derived from the current window dimensions. That view will be updated explicitly when the window size changes and used for the silhouette composite pass and other shell-owned overlays.

This is preferred over relying on `window.getDefaultView()` because the resize bug strongly suggests the final composite step is using stale or mismatched screen-space mapping. An explicit view makes the presentation contract visible in code and keeps overlay behavior deterministic after resize.

Alternative considered: continue using the default view and try to repair the final silhouette sprite placement manually. Rejected because it keeps the bug-prone implicit dependency and duplicates placement logic that should remain shared with the player mask pass.

### Preserve world-space rendering for the mask generation passes

The player occlusion mask texture and occluder mask texture will continue to render with the same world view used for normal terrain and object drawing. This keeps the silhouette derived from the same player transform, world camera frame, and occluder ordering as the visible scene.

Alternative considered: render and position the silhouette as an independently placed world sprite. Rejected because it introduces a second placement path that must stay perfectly synchronized with the player sprite's origin, scale, and position.

### Cover the bug with resize-focused shell tests

The change should extend runtime-facing tests to cover the resize case where the silhouette remains aligned after the viewport changes. Existing tests already cover viewport propagation and shader overlap math, but they do not cover the final composite contract after resize.

Alternative considered: rely only on manual verification. Rejected because the defect is tied to a specific render-path boundary that should stay guarded once fixed.

## Risks / Trade-offs

- **Overlay view ownership adds another runtime presentation object** -> Keep the new view limited to shell-owned screen-space passes and reuse it for overlays that already draw outside the world view.
- **Tests may be harder to express than pure gameplay snapshot tests** -> Focus them on observable shell contracts such as resize-aware view updates and silhouette alignment invariants, rather than on fragile implementation details.
- **The defect could involve SFML default-view behavior that varies by backend** -> Avoid backend-specific assumptions by making resize handling explicit in the runtime shell.
