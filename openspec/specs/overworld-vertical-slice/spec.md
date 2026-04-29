## Purpose

Define the deterministic overworld vertical slice, including generation, movement, camera behavior, and basic rendering.

## Requirements

### Requirement: Deterministic overworld generation
The runtime SHALL generate a finite overworld tile grid from a built-in deterministic seed so the resulting terrain layout is the same on every run of the same build.

#### Scenario: Stable map layout on startup
- **WHEN** the game runtime creates a new overworld slice
- **THEN** the world module generates the tile grid from the built-in deterministic seed
- **AND** repeated launches produce the same tile categories at the same coordinates

#### Scenario: Playable spawn selection
- **WHEN** the world finishes generating the overworld slice
- **THEN** it provides at least one player spawn position inside the world bounds
- **AND** that spawn position is placed on a traversable tile

### Requirement: Player movement within the overworld slice
The runtime SHALL place the player in the generated overworld and update the player's position in response to configured movement input while keeping the player inside valid traversable space for the slice.

#### Scenario: Initial player placement
- **WHEN** a new overworld slice starts
- **THEN** the player is placed at the world-provided spawn position

#### Scenario: Traversing a valid tile
- **WHEN** the player provides movement input toward an in-bounds traversable location
- **THEN** the player position advances in that direction during update processing

#### Scenario: Rejecting invalid movement
- **WHEN** the player provides movement input toward an out-of-bounds or non-traversable location
- **THEN** the runtime keeps the player in the nearest valid position inside the slice

### Requirement: Camera tracks the player in the overworld
The runtime SHALL frame the overworld through the camera module so the view follows the player's current position while remaining clamped to the world bounds.

#### Scenario: Camera follows active player position
- **WHEN** the player position changes during the overworld slice
- **THEN** the camera updates its framing from the player's current position

#### Scenario: Camera respects world edges
- **WHEN** the player approaches the edge of the generated world
- **THEN** the camera remains within the valid world extents
- **AND** the rendered view does not expose space outside the overworld bounds

### Requirement: Basic tile rendering for the overworld slice
The runtime SHALL render the generated overworld and the player marker each frame using basic built-in visuals that distinguish at least the traversable terrain from non-traversable terrain.

#### Scenario: Rendering the generated map
- **WHEN** the runtime renders a frame for the overworld slice
- **THEN** it draws the visible portion of the generated tile grid
- **AND** each drawn tile uses a visual representation derived from its tile category

#### Scenario: Rendering the player in the world view
- **WHEN** the runtime renders a frame after the player has been placed in the overworld
- **THEN** it draws a visible player marker at the player's current world position using the active camera framing
