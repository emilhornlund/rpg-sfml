## ADDED Requirements

### Requirement: Screen-space overlay presentation follows the active window size
The game runtime SHALL maintain explicit screen-space presentation state for shell-owned overlay rendering that depends on window pixel dimensions. When the active window size changes, the runtime SHALL update that screen-space presentation state before drawing later overlay passes so resize-sensitive overlays do not continue rendering against stale screen coordinates.

#### Scenario: Resize updates later overlay composition
- **WHEN** the player resizes the application window during active gameplay
- **THEN** the runtime updates the screen-space presentation state used for later shell-owned overlay passes
- **AND** subsequent overlay drawing uses the resized window dimensions instead of the previous window size

#### Scenario: World-space and screen-space presentation remain distinct
- **WHEN** the runtime renders an overworld frame that includes both world-space scene drawing and shell-owned overlays
- **THEN** terrain, generated content, and gameplay markers continue to use the published overworld camera frame
- **AND** shell-owned overlay composition uses explicit screen-space presentation state derived from the active window size
