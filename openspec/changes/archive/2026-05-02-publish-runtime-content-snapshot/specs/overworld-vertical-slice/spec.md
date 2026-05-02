## MODIFIED Requirements

### Requirement: Basic tile rendering for the overworld slice
The runtime SHALL render the generated overworld, generated world content, and the player each frame using built-in terrain visuals, deterministic content presentation, and sprite-backed player presentation, and it SHALL derive terrain and generated-content drawing from camera-visible world traversal instead of full-world tile or chunk loops. The player presentation SHALL align to the 16x16 world tile baseline while allowing the visible character art to extend upward beyond a single tile.

#### Scenario: Rendering the generated map
- **WHEN** the runtime renders a frame for the overworld
- **THEN** it draws the visible portion of the generated tile grid that intersects the active camera framing
- **AND** each drawn tile uses a visual representation derived from its tile category

#### Scenario: Rendering newly generated terrain
- **WHEN** the active camera frame reaches chunks that were not previously retained by the world
- **THEN** the runtime renders tiles from those newly generated chunks during that frame

#### Scenario: Rendering visible generated world content
- **WHEN** the runtime renders a frame whose active camera framing intersects generated world content
- **THEN** it draws that visible generated content from the runtime-published render snapshot
- **AND** the shell does not query `World` directly to discover what generated content should be drawn

#### Scenario: Rendering the player in the world view
- **WHEN** the runtime renders a frame after the player has been placed in the overworld
- **THEN** it draws the player from the active walking spritesheet using the active camera framing
- **AND** the player remains aligned to the occupied world tile through the sprite pivot rather than through a generic marker rectangle

### Requirement: Render-facing overworld frame state
The runtime SHALL expose render-facing overworld frame state through the dedicated overworld runtime boundary so the SFML shell can render the current frame without deriving visible terrain traversal, generated-content visibility, and player marker placement directly from gameplay modules.

#### Scenario: Rendering from overworld frame state
- **WHEN** the SFML shell renders the active overworld frame
- **THEN** it consumes camera framing, visible terrain data, visible generated-content data, and player marker state supplied by the dedicated overworld runtime
- **AND** `Game.cpp` does not assemble those values directly from `World`, `Player`, and `Camera`
