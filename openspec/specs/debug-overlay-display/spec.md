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
The debug overlay SHALL render as a screen-anchored semi-transparent black panel with readable text showing frame rate, retained chunk count, retained generated-content count, rendered generated-content count, player tile coordinates, and zoom level.

#### Scenario: Drawing overlay diagnostics
- **WHEN** the debug overlay is visible during an overworld frame
- **THEN** the runtime draws a semi-transparent black background panel in screen space
- **AND** it renders text for frame rate, retained chunk count, retained generated-content count, rendered generated-content count, player tile coordinates, and zoom level on top of that panel

#### Scenario: Overlay reflects current cache state after chunk unloading
- **WHEN** the player explores far enough for the world cache to unload out-of-window chunks
- **THEN** the overlay diagnostics reflect the current retained chunk count and retained generated-content count after unloading
- **AND** the rendered generated-content count continues to describe only the objects visible in the active frame

#### Scenario: Overlay hides cleanly
- **WHEN** the debug overlay is toggled off
- **THEN** the runtime stops drawing the overlay panel and diagnostic text
- **AND** overworld scene rendering continues without leaving overlay artifacts on screen
