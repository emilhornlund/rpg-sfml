## ADDED Requirements

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
