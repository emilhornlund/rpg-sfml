## MODIFIED Requirements

### Requirement: Deterministic overworld generation
The runtime SHALL generate the overworld from explicit world configuration inputs that include a deterministic seed, tile size, and any finite-slice sizing inputs still required by the current vertical slice so the resulting terrain queries, spawn point, and world-facing dimensions remain stable for a given configuration. The generated terrain SHALL derive its tile categories from deterministic layered environmental signals that include elevation and moisture so neighboring tiles form spatially coherent landforms and biomes rather than independent per-tile noise. The world implementation SHALL be allowed to satisfy those deterministic terrain queries through chunk-addressed generated data retained behind `World` instead of requiring one eagerly generated full-world tile grid.

#### Scenario: Stable map layout for the same configuration
- **WHEN** the game runtime creates two overworld slices with the same world configuration
- **THEN** each world module generates the same tile categories at the same coordinates
- **AND** each world reports the same spawn position, world dimensions, and tile size

#### Scenario: World dimensions follow configuration
- **WHEN** the game runtime creates an overworld slice with a configured width, height, and tile size
- **THEN** the world module reports those configured dimensions through its world-size and tile-grid accessors
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
