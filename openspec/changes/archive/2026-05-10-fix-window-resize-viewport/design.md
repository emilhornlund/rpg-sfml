## Context

The current runtime creates the SFML window at a fixed startup size and continues to publish that startup viewport into the overworld runtime on every frame. Rendering itself already consumes the camera frame from gameplay-owned render snapshots, so the visible stretching happens because gameplay camera sizing is driven by stale viewport dimensions rather than by the live window size.

This change stays within the existing runtime boundaries. `Game` remains the SFML-facing shell, `OverworldRuntime` remains responsible for camera and snapshot assembly, and the debug overlay continues to render in screen space through the default view.

## Goals / Non-Goals

**Goals:**
- Keep overworld camera sizing synchronized with the current window dimensions after a resize.
- Preserve the current `Game` → `OverworldRuntime` → `Camera` ownership chain for viewport and camera-frame calculation.
- Prevent stretched or squeezed overworld rendering without changing existing debug overlay behavior.

**Non-Goals:**
- Introduce letterboxing, pillarboxing, or any fixed-aspect presentation policy.
- Redesign the camera zoom model or the render snapshot format beyond the viewport-sizing behavior needed for this fix.
- Move resize handling responsibilities into gameplay modules or expose SFML types beyond the shell boundary.

## Decisions

### Use live window size as the viewport source

The runtime shell will derive the overworld viewport from the active SFML window size for each frame instead of reusing the startup constants after window creation.

**Rationale:** The camera frame is already recalculated from repo-native viewport dimensions during overworld updates. Feeding the current window size through the existing input boundary fixes the distortion at the source and keeps the rest of the runtime flow unchanged.

**Alternatives considered:**
- Handle resize only by calling `window.setView(...)` in the event loop. Rejected because the main render path already overwrites the window view from snapshot-owned camera data each frame.
- Store resize state separately and only refresh viewport data on resize events. Rejected because querying the current window size each frame is simpler, matches the existing update loop, and avoids stale state if other window-size changes occur.

### Keep resize effects inside existing snapshot and camera flow

No new top-level resize subsystem is needed. `Game` continues translating shell-owned state into repo-native overworld input, and `OverworldRuntime` continues to publish the camera frame through render snapshots.

**Rationale:** The bug is caused by stale viewport input, not by a missing rendering layer. Reusing the current flow keeps the change narrow and aligned with existing module boundaries.

**Alternatives considered:**
- Add a dedicated resize-specific runtime collaborator. Rejected because it would add indirection without solving a broader architectural problem.

### Preserve screen-space debug overlay behavior

The debug overlay remains anchored to the default view after world rendering, with no change to its content or toggle semantics.

**Rationale:** The overlay already behaves like a shell-owned screen-space layer and is not part of the distortion problem.

## Risks / Trade-offs

- **Dynamic viewport reveals more or less world as the window size changes** → This is an intentional consequence of using live window dimensions instead of fixed-aspect letterboxing.
- **Tests that assume the startup viewport remains constant after initialization may fail** → Update focused runtime and snapshot tests to assert the resized viewport path explicitly.
- **Very small window sizes may produce very small camera frames** → Continue relying on existing SFML window sizing behavior and current camera math; this change does not add new minimum-size policy.
