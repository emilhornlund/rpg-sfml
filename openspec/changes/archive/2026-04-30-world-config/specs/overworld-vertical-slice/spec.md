## MODIFIED Requirements

### Requirement: Deterministic overworld generation
The runtime SHALL generate a finite overworld tile grid from explicit world configuration inputs that include a deterministic seed, tile-grid width, tile-grid height, and tile size so the resulting terrain layout and world dimensions are stable for a given configuration.

#### Scenario: Stable map layout for the same configuration
- **WHEN** the game runtime creates two overworld slices with the same world configuration
- **THEN** each world module generates the same tile categories at the same coordinates
- **AND** each world reports the same spawn position, world dimensions, and tile size

#### Scenario: World dimensions follow configuration
- **WHEN** the game runtime creates an overworld slice with a configured width, height, and tile size
- **THEN** the world module reports those configured dimensions through its world-size and tile-grid accessors
- **AND** coordinate conversion and tile-center lookups use the configured tile size

#### Scenario: Playable spawn selection
- **WHEN** the world finishes generating the overworld slice from a world configuration
- **THEN** it provides at least one player spawn position inside the configured world bounds
- **AND** that spawn position is placed on a traversable tile

#### Scenario: Built-in default world configuration
- **WHEN** the runtime creates a world without providing explicit configuration inputs
- **THEN** the world module uses the built-in default configuration
- **AND** the generated overworld remains deterministic across repeated launches of the same build
