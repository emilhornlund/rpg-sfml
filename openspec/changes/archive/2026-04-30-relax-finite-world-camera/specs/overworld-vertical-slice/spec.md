## MODIFIED Requirements

### Requirement: Deterministic overworld generation
The runtime SHALL generate the overworld from explicit world configuration inputs that include a deterministic seed, tile size, and any finite-slice sizing inputs still required by the current vertical slice so the resulting terrain queries, spawn point, and generated slice extent remain stable for a given configuration. The generated terrain SHALL derive its tile categories from deterministic layered environmental signals that include elevation and moisture so neighboring tiles form spatially coherent landforms and biomes rather than independent per-tile noise. The world implementation SHALL be allowed to satisfy those deterministic terrain queries through chunk-addressed generated data retained behind `World` instead of requiring one eagerly generated full-world tile grid, and camera-facing runtime flows SHALL NOT require world-size accessors in order to frame the view.

#### Scenario: Stable map layout for the same configuration
- **WHEN** the game runtime creates two overworld slices with the same world configuration
- **THEN** each world module generates the same tile categories at the same coordinates
- **AND** each world reports the same spawn position and tile size

#### Scenario: World configuration drives the generated slice extent
- **WHEN** the game runtime creates an overworld slice with a configured width, height, and tile size
- **THEN** the generated terrain occupies the finite slice implied by that configuration
- **AND** coordinate conversion and tile-center lookups use the configured tile size

#### Scenario: Water forms the world boundary
- **WHEN** the world finishes generating the overworld slice
- **THEN** the outer border of the finite tile grid is classified as water
- **AND** the interior terrain transitions away from that boundary through the deterministic terrain-generation rules

#### Scenario: Shoreline tiles form from low-elevation land
- **WHEN** generated land tiles fall within the shoreline elevation band above water
- **THEN** the world classifies those tiles as sand
- **AND** inland tiles are not classified as sand solely from independent per-tile randomness

#### Scenario: Inland biome cover depends on moisture
- **WHEN** generated tiles are above the shoreline elevation band
- **THEN** the world classifies wetter inland tiles as forest
- **AND** drier inland tiles remain grass

#### Scenario: Playable spawn selection
- **WHEN** the world finishes generating the overworld slice from a world configuration
- **THEN** it provides at least one player spawn position inside the configured world bounds
- **AND** that spawn position is placed on a traversable tile

#### Scenario: Built-in default world configuration
- **WHEN** the runtime creates a world without providing explicit configuration inputs
- **THEN** the world module uses the built-in default configuration
- **AND** the generated overworld remains deterministic across repeated launches of the same build

### Requirement: Camera tracks the player in the overworld
The runtime SHALL frame the overworld through the camera module so the view follows the player's current position without clamping the camera frame to generated world bounds.

#### Scenario: Camera follows active player position
- **WHEN** the player position changes during the overworld slice
- **THEN** the camera updates its framing from the player's current position

#### Scenario: Camera can move beyond world edges
- **WHEN** the player approaches the edge of the generated world
- **THEN** the camera frame remains centered on the tracked focus position instead of being clamped to the world extents
- **AND** the rendered view may expose space outside the generated overworld bounds

### Requirement: Basic tile rendering for the overworld slice
The runtime SHALL render the generated overworld and the player marker each frame using basic built-in visuals that distinguish at least the traversable terrain from non-traversable terrain.

#### Scenario: Rendering the generated map
- **WHEN** the runtime renders a frame for the overworld slice
- **THEN** it draws the visible portion of the generated tile grid that intersects the active camera framing
- **AND** each drawn tile uses a visual representation derived from its tile category

#### Scenario: Rendering space beyond generated terrain
- **WHEN** the active camera frame extends beyond the generated overworld near a map edge
- **THEN** the runtime draws only generated terrain tiles that intersect that frame
- **AND** any remaining view area outside the generated terrain is left as non-terrain background

#### Scenario: Rendering the player in the world view
- **WHEN** the runtime renders a frame after the player has been placed in the overworld
- **THEN** it draws a visible player marker at the player's current world position using the active camera framing
