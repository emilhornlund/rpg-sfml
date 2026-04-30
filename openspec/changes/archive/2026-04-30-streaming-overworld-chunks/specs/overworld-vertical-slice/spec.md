## MODIFIED Requirements

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

### Requirement: Player movement within the overworld slice
The runtime SHALL place the player in the generated overworld and update the player's position in response to configured movement input while keeping the player inside valid traversable space provided by the streaming terrain system.

#### Scenario: Initial player placement
- **WHEN** a new overworld session starts
- **THEN** the player is placed at the world-provided spawn position

#### Scenario: Traversing a valid tile
- **WHEN** the player provides movement input toward a traversable location
- **THEN** the player position advances in that direction during update processing

#### Scenario: Rejecting invalid movement
- **WHEN** the player provides movement input toward a non-traversable location
- **THEN** the runtime keeps the player in the nearest valid position

### Requirement: Basic tile rendering for the overworld slice
The runtime SHALL render the generated overworld and the player marker each frame using basic built-in visuals that distinguish at least the traversable terrain from non-traversable terrain, and it SHALL derive terrain drawing from camera-visible terrain traversal instead of full-world tile loops.

#### Scenario: Rendering the generated map
- **WHEN** the runtime renders a frame for the overworld
- **THEN** it draws the visible portion of the generated tile grid that intersects the active camera framing
- **AND** each drawn tile uses a visual representation derived from its tile category

#### Scenario: Rendering newly generated terrain
- **WHEN** the active camera frame reaches chunks that were not previously retained by the world
- **THEN** the runtime renders tiles from those newly generated chunks during that frame

#### Scenario: Rendering the player in the world view
- **WHEN** the runtime renders a frame after the player has been placed in the overworld
- **THEN** it draws a visible player marker at the player's current world position using the active camera framing
