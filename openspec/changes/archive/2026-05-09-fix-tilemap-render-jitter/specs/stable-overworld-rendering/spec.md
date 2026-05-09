## ADDED Requirements

### Requirement: Overworld runtime preserves responsive frame-variable movement
The game runtime SHALL continue to process overworld simulation from the elapsed wall-clock delta of the current display frame so player movement and walk animation cadence remain responsive. The runtime SHALL still process frames through distinct event, update, and render phases in that order.

#### Scenario: Frame advances through the existing lifecycle
- **WHEN** the main window processes a display frame while gameplay is active
- **THEN** the game runtime processes events once for that frame
- **AND** it advances the overworld simulation from the current frame delta
- **AND** it renders one overworld frame after the update

### Requirement: Overworld camera presentation is pixel-stable
The render-facing camera frame used for overworld terrain presentation SHALL be aligned to the world pixel grid before the SFML view is applied so visible terrain tiles do not shimmer when the player moves or the debug zoom changes. This requirement applies to the camera frame used for rendering, not to the underlying gameplay-facing tracking position.

#### Scenario: Fractional gameplay movement does not cause terrain shimmer
- **WHEN** the player advances through the overworld with world-space positions between whole pixels
- **THEN** the camera may continue to track the gameplay position internally
- **AND** the camera frame used for rendering is snapped to pixel-aligned coordinates before terrain is drawn
- **AND** visible terrain tiles remain visually stable relative to the screen during movement

#### Scenario: Zoomed-out view remains aligned
- **WHEN** the overworld is rendered at a zoom level that shows many tiles on screen
- **THEN** the render-facing camera frame remains pixel-aligned
- **AND** the view does not introduce additional sub-pixel tilemap shimmer due to fractional camera centers

### Requirement: Camera snapping is derived from the active zoom
The render-facing camera frame SHALL be snapped in world-space increments derived from the active camera frame size and the current viewport dimensions instead of rounding the camera center to whole world units. This preserves pixel-stable terrain rendering while reducing visible camera lag at non-default zoom levels.

#### Scenario: Zoom-aware snap step follows the viewport pixel grid
- **WHEN** the overworld camera publishes a render-facing frame for the active viewport
- **THEN** the runtime derives the snap increment from the current camera frame size and viewport dimensions
- **AND** the snapped camera center advances in screen-pixel-sized world-space steps instead of coarse whole-world-unit steps
- **AND** the snapped frame keeps the camera visually closer to the player than whole-world-unit snapping would

### Requirement: Runtime uses one display pacing strategy at a time
The game runtime SHALL not enable vertical sync and an active framerate limit at the same time for the main overworld window. The configured pacing strategy SHALL be explicit so display pacing behavior is not split across competing limiters.

#### Scenario: Vertical sync disables the framerate limiter
- **WHEN** the main window is configured to use vertical sync
- **THEN** the runtime does not also apply a nonzero framerate limit to the same window

#### Scenario: Framerate limiter is used as fallback pacing
- **WHEN** the main window is not configured to use vertical sync
- **THEN** the runtime may apply an explicit framerate limit as the active fallback pacing strategy
- **AND** that limiter is the only active frame pacing control owned by the game shell

### Requirement: Render snapshot uses one consistent presentation camera
The render-facing camera frame published by the overworld runtime SHALL be the same frame used both for visibility collection and for configuring the SFML view for the frame. The runtime SHALL not collect visible terrain or generated content from one camera frame and then draw using a different presentation camera.

#### Scenario: Visibility and drawing share the same camera frame
- **WHEN** the overworld runtime publishes a render snapshot for the current frame
- **THEN** the visible terrain and generated content are collected from the published render-facing camera frame
- **AND** the outer game shell uses that same published frame to configure the SFML view before drawing
- **AND** tile borders do not flash due to mismatched culling and draw cameras
