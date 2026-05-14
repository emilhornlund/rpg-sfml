## Purpose

Define the runtime-visible debug overlay behavior and diagnostics presentation for active overworld play.

## Requirements

### Requirement: Runtime can show and hide a debug overlay during play
The game runtime SHALL provide a debug overlay that can be toggled on and off during active overworld play without pausing the frame loop or blocking movement and other debug controls.

#### Scenario: Toggling overlay visibility
- **WHEN** the player presses `F1` during active gameplay
- **THEN** the runtime toggles the debug overlay visibility for subsequent frames
- **AND** the frame loop continues running without entering a paused or modal state

#### Scenario: Overlay remains non-blocking while visible
- **WHEN** the debug overlay is visible and the player presses movement or debug-view controls
- **THEN** the runtime continues to process those controls using the normal frame lifecycle
- **AND** overlay visibility does not suppress movement, zoom, or tile-grid behavior

### Requirement: Debug overlay presents readable runtime diagnostics
The debug overlay SHALL render as a screen-anchored semi-transparent black panel with readable text showing frame rate, retained chunk count, retained generated-content count, visible tile count, visible generated-content count, front occluder count, overlap-qualified occlusion candidate count, terrain vertex count, tile-grid vertex count, player tile coordinates, and zoom level.

#### Scenario: Drawing overlay diagnostics
- **WHEN** the debug overlay is visible during an overworld frame
- **THEN** the runtime draws a semi-transparent black background panel in screen space
- **AND** it renders text for frame rate, retained chunk count, retained generated-content count, visible tile count, visible generated-content count, front occluder count, overlap-qualified occlusion candidate count, terrain vertex count, tile-grid vertex count, player tile coordinates, and zoom level on top of that panel

#### Scenario: Overlay reflects current cache state after chunk unloading
- **WHEN** the player explores far enough for the world cache to unload out-of-window chunks
- **THEN** the overlay diagnostics reflect the current retained chunk count and retained generated-content count after unloading
- **AND** the visible generated-content count continues to describe only the objects visible in the active frame

#### Scenario: Overlay reports occlusion and render-batch scale
- **WHEN** the debug overlay is visible for an overworld frame with terrain, generated content, and player-occlusion diagnostics prepared for drawing
- **THEN** the overlay reports the visible tile count and terrain vertex count for the active frame
- **AND** it reports the broad front occluder count derived from the current render queue
- **AND** it reports the overlap-qualified occlusion candidate count derived from the active player-overlap test
- **AND** it reports the tile-grid vertex count for the current frame, using zero when the tile-grid overlay is disabled

#### Scenario: Overlay hides cleanly
- **WHEN** the debug overlay is toggled off
- **THEN** the runtime stops drawing the overlay panel and diagnostic text
- **AND** overworld scene rendering continues without leaving overlay artifacts on screen

### Requirement: Tile-grid visualization stays aligned while batched
The game runtime SHALL preserve the existing debug tile-grid visualization semantics when tile-grid rendering is submitted as batched geometry, including the current toggle behavior, world-space alignment, and screen-clean removal when the grid is disabled.

#### Scenario: Batched grid respects the existing toggle
- **WHEN** the player enables or disables the tile-grid overlay through the debug view controls during active overworld play
- **THEN** the runtime toggles the tile-grid visualization for subsequent frames using the same control path as before
- **AND** batching the grid submission does not change whether the overlay is shown or hidden

#### Scenario: Batched grid remains aligned across camera changes
- **WHEN** the tile-grid overlay is visible while the player moves, the viewport is resized, or the debug zoom level changes
- **THEN** the grid visualization remains aligned to the visible overworld tile boundaries for the active frame
- **AND** disabling the grid removes the visualization cleanly without leaving stale batched geometry on screen

### Requirement: Tile-grid batching avoids duplicated interior edge geometry
The game runtime SHALL build the debug tile-grid overlay from the rectangular bounds implied by the visible tile set instead of expanding each visible tile into four independent edge rectangles.

#### Scenario: Grid overlay emits one strip per visible boundary
- **WHEN** debug view enables the tile-grid overlay for an overworld frame with `W` visible tile columns and `H` visible tile rows
- **THEN** the render shell builds `W + 1` vertical strips and `H + 1` horizontal strips for that frame
- **AND** it does not rebuild every visible tile edge as a separate rectangle

#### Scenario: Grid overlay preserves current world-space appearance
- **WHEN** the optimized tile-grid overlay is visible during movement, resize, or debug zoom changes
- **THEN** the grid remains aligned to the active visible tile boundaries in world space
- **AND** the outer border keeps the configured line thickness while interior boundaries remain visually equivalent to the current doubled-edge presentation

#### Scenario: Grid overlay metrics reflect optimized geometry
- **WHEN** the debug overlay is visible while the tile-grid overlay is enabled
- **THEN** the reported tile-grid vertex count reflects the bounds-derived strip geometry for the active frame
- **AND** disabling the tile-grid overlay resets the reported tile-grid vertex count to zero for subsequent frames
