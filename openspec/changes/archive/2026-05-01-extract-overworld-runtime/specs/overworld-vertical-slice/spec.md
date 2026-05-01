## ADDED Requirements

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

### Requirement: Render-facing overworld frame state
The runtime SHALL expose render-facing overworld frame state through the dedicated overworld runtime boundary so the SFML shell can render the current frame without deriving visible terrain traversal and player marker placement directly from gameplay modules.

#### Scenario: Rendering from overworld frame state
- **WHEN** the SFML shell renders the active overworld frame
- **THEN** it consumes camera framing, visible terrain data, and player marker state supplied by the dedicated overworld runtime
- **AND** `Game.cpp` does not assemble those values directly from `World`, `Player`, and `Camera`
