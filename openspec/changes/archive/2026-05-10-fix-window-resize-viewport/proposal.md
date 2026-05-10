## Why

Resizing the game window currently stretches or squeezes the overworld instead of preserving the intended framing. The runtime keeps publishing the original startup viewport size after the window is resized, so the rendered scene no longer matches the active window dimensions.

## What Changes

- Update the runtime shell to derive the active overworld viewport from the current window size instead of the fixed startup constants after the window has been created.
- Define resize behavior so overworld camera framing and render snapshots stay aligned with the active window dimensions across subsequent frames.
- Preserve the existing screen-space debug overlay behavior while the gameplay view responds to window resizing.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `game-runtime`: the runtime shell updates gameplay viewport sizing from the live window state during play, including after resize events.
- `render-snapshots`: published camera-frame sizing stays synchronized with the active window dimensions so rendered overworld content is not stretched by stale viewport data.

## Impact

- Affected code: `src/main/Game.cpp`, `src/main/OverworldRuntime.cpp`, `src/main/Camera.cpp`, and related runtime support/tests.
- Affected behavior: overworld framing during window resize, camera snapshot sizing, and any tests that assume a fixed runtime viewport after startup.
- No new external dependencies or public API surfaces are expected.
