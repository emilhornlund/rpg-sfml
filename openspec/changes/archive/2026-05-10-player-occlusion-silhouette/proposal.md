## Why

Dense foreground vegetation can completely hide the player, making navigation and positioning hard to read. The intended solution is not tree staging or canopy splitting, but a partial silhouette that reveals only the portion of the player sprite actually occluded by front objects while preserving normal y-sorted world rendering.

## What Changes

- Add a player-occlusion silhouette effect that shows a faded/tinted version of only the hidden portion of the player when front objects cover them.
- Preserve the existing global y-sorted overworld object rendering for terrain, generated content, and the player.
- Derive silhouette occlusion from front objects in the render order instead of introducing tree-specific rules or requiring new artwork.
- Keep the visible portion of the player sprite unchanged; only the hidden portion gains the silhouette treatment.

## Capabilities

### New Capabilities
- None.

### Modified Capabilities
- `generated-content-rendering`: Change overworld object presentation so front occluders can reveal a partial player silhouette without breaking the normal y-sorted world layer.

## Impact

- Affected specs: `generated-content-rendering`
- Affected code: `src/main/Game.cpp`, render helpers in `src/main/GameRuntimeSupport.hpp`, and related tests
- No gameplay, asset, or dependency changes are required by the proposal itself
