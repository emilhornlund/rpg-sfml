## MODIFIED Requirements

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
