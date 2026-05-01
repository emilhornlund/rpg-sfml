## Purpose

Define the deterministic overworld vertical slice, including generation, movement, camera behavior, and basic rendering.

## Requirements

### Requirement: Deterministic overworld generation
The runtime SHALL generate the overworld from explicit world configuration inputs that include a deterministic seed and tile size, plus any compatibility sizing inputs retained during migration, so the resulting terrain queries and spawn point remain stable for a given configuration. The generated terrain SHALL derive its tile categories from deterministic layered environmental signals that include elevation and moisture so neighboring tiles form spatially coherent landforms and biomes rather than independent per-tile noise. The world implementation SHALL satisfy deterministic terrain queries through chunk-addressed generated data retained behind `World` instead of requiring one eagerly generated full-world tile grid, and camera-facing runtime flows SHALL NOT require world-size accessors in order to frame the view.

#### Scenario: Stable map layout for the same configuration
- **WHEN** the game runtime creates two overworld instances with the same world configuration
- **THEN** each world module generates the same tile categories at the same coordinates
- **AND** each world reports the same spawn position and tile size

#### Scenario: Terrain generation extends beyond the initial player vicinity
- **WHEN** the game runtime queries terrain outside the chunks retained during initial world setup
- **THEN** the world generates deterministic terrain for those additional chunks
- **AND** the terrain classification does not depend on a finite world edge

#### Scenario: Shoreline tiles form from low-elevation land
- **WHEN** generated land tiles fall within the shoreline elevation band above water
- **THEN** the world classifies those tiles as sand
- **AND** inland tiles are not classified as sand solely from independent per-tile randomness

#### Scenario: Inland biome cover depends on moisture
- **WHEN** generated tiles are above the shoreline elevation band
- **THEN** the world classifies wetter inland tiles as forest
- **AND** drier inland tiles remain grass

#### Scenario: Playable spawn selection
- **WHEN** the world initializes the overworld from a world configuration
- **THEN** it provides at least one player spawn position at or near a deterministic world anchor
- **AND** that spawn position is placed on a traversable tile

#### Scenario: Built-in default world configuration
- **WHEN** the runtime creates a world without providing explicit configuration inputs
- **THEN** the world module uses the built-in default configuration
- **AND** the generated overworld remains deterministic across repeated launches of the same build

### Requirement: Dedicated overworld runtime coordination
The runtime SHALL coordinate overworld session initialization, player input application, player/world/camera update sequencing, and render-facing frame assembly through a dedicated overworld runtime collaborator outside the SFML shell.

#### Scenario: Starting a new overworld session
- **WHEN** a new overworld session begins
- **THEN** the dedicated overworld runtime initializes the player's spawn state from the world-owned spawn data
- **AND** it establishes the camera framing from the active gameplay state

#### Scenario: Advancing the overworld from gameplay input
- **WHEN** the runtime receives the current frame's overworld input
- **THEN** the dedicated overworld runtime applies that input to the overworld gameplay state during update processing
- **AND** it advances player and camera state without requiring `Game.cpp` to sequence those gameplay modules directly

### Requirement: Player movement within the overworld slice
The runtime SHALL place the player in the generated overworld and update the player's position in response to configured movement input while keeping the player on tile-centered cardinal movement paths inside valid traversable space provided by the streaming terrain system. Once a tile step begins, the runtime SHALL continue that step to the destination tile center before stopping or choosing a new step.

#### Scenario: Initial player placement
- **WHEN** a new overworld session starts
- **THEN** the player is placed at the world-provided spawn position

#### Scenario: Traversing a valid tile
- **WHEN** the player provides movement input toward a traversable adjacent tile
- **THEN** the player position advances along the straight segment between the current tile center and that adjacent tile center
- **AND** the player finishes the step at the destination tile center

#### Scenario: Resolving perpendicular input during movement
- **WHEN** the player holds a new perpendicular direction while already traveling toward the current destination tile center
- **THEN** the current tile step continues unchanged until that destination center is reached
- **AND** any new step started after that point uses the active resolved cardinal direction for the next tile

#### Scenario: Completing a started step after input release
- **WHEN** movement input is released after the player has started moving toward an adjacent traversable tile
- **THEN** the runtime continues advancing the player to that tile's center
- **AND** the player stops there if no active movement direction remains

#### Scenario: Rejecting invalid movement
- **WHEN** the player provides movement input toward a non-traversable adjacent tile while idle at a tile center
- **THEN** the runtime does not begin a new tile step
- **AND** the player remains at the current tile center

### Requirement: Camera tracks the player in the overworld
The runtime SHALL frame the overworld through the camera module so the view follows the player's current position without clamping the camera frame to an initially retained terrain area.

#### Scenario: Camera follows active player position
- **WHEN** the player position changes during the overworld slice
- **THEN** the camera updates its framing from the player's current position

#### Scenario: Camera can move beyond the initial retained area
- **WHEN** the player approaches the edge of the terrain retained during initial world setup
- **THEN** the camera frame remains centered on the tracked focus position instead of being clamped to that initial retained area
- **AND** the rendered view may extend into terrain that will be generated on demand

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

### Requirement: Basic tile rendering for the overworld slice
The runtime SHALL render the generated overworld and the player each frame using built-in terrain visuals and sprite-backed player presentation, and it SHALL derive terrain drawing from camera-visible terrain traversal instead of full-world tile loops. The player presentation SHALL align to the 16x16 world tile baseline while allowing the visible character art to extend upward beyond a single tile.

#### Scenario: Rendering the generated map
- **WHEN** the runtime renders a frame for the overworld
- **THEN** it draws the visible portion of the generated tile grid that intersects the active camera framing
- **AND** each drawn tile uses a visual representation derived from its tile category

#### Scenario: Rendering newly generated terrain
- **WHEN** the active camera frame reaches chunks that were not previously retained by the world
- **THEN** the runtime renders tiles from those newly generated chunks during that frame

#### Scenario: Rendering the player in the world view
- **WHEN** the runtime renders a frame after the player has been placed in the overworld
- **THEN** it draws the player from the active walking spritesheet using the active camera framing
- **AND** the player remains aligned to the occupied world tile through the sprite pivot rather than through a generic marker rectangle

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

### Requirement: Render-facing overworld frame state
The runtime SHALL expose render-facing overworld frame state through the dedicated overworld runtime boundary so the SFML shell can render the current frame without deriving visible terrain traversal and player marker placement directly from gameplay modules.

#### Scenario: Rendering from overworld frame state
- **WHEN** the SFML shell renders the active overworld frame
- **THEN** it consumes camera framing, visible terrain data, and player marker state supplied by the dedicated overworld runtime
- **AND** `Game.cpp` does not assemble those values directly from `World`, `Player`, and `Camera`
