## ADDED Requirements

### Requirement: Debug zoom reframes the overworld camera
The overworld runtime SHALL default the active camera to a 300% zoom level around the tracked player position so the baseline view is appropriate for 16x16 world tiles. During development and testing, the runtime SHALL additionally support debug zoom hotkeys that adjust that framing in 50% increments within the supported debug range.

#### Scenario: Starting the overworld
- **WHEN** a new overworld session begins
- **THEN** the active camera frame is centered on the player's tracked position
- **AND** the frame size reflects the default 300% zoom instead of the unscaled window-sized view

#### Scenario: Adjusting the debug zoom level
- **WHEN** the player triggers a debug zoom hotkey during development or testing
- **THEN** the next published camera frame uses the new zoom-adjusted size while remaining centered on the tracked player position
- **AND** visible terrain for that frame continues to be derived from the published camera frame

### Requirement: Debug zoom scales traversal pace
During development and testing, the overworld runtime SHALL scale player movement speed relative to the active debug zoom so traversal keeps a roughly stable on-screen pace across supported debug zoom levels.

#### Scenario: Zooming out increases traversal speed
- **WHEN** the active debug zoom level shows more world area than the default debug zoom
- **THEN** the player's world-space movement speed increases relative to the default debug movement speed
- **AND** tile-step traversal remains tile-centered and cardinal

#### Scenario: Zooming in decreases traversal speed
- **WHEN** the active debug zoom level shows less world area than the default debug zoom
- **THEN** the player's world-space movement speed decreases relative to the default debug movement speed
- **AND** existing traversability and step-completion rules still apply

### Requirement: Debug tile-grid overlay renders between terrain and player
During development and testing, the runtime SHALL support a debug-only tile-grid overlay that can be toggled while the overworld is running. When enabled, the overlay SHALL render above terrain tiles and below the player presentation.

#### Scenario: Grid overlay is enabled
- **WHEN** the debug tile-grid overlay is toggled on during development or testing
- **THEN** the rendered overworld frame includes grid lines aligned to the visible tile geometry
- **AND** those grid lines appear above terrain rendering and below the player rendering

#### Scenario: Grid overlay is disabled
- **WHEN** the debug tile-grid overlay is toggled off during development or testing
- **THEN** the rendered overworld frame omits the grid overlay
- **AND** terrain and player rendering continue unchanged
