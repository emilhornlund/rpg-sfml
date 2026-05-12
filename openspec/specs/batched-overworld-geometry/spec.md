## Purpose

Define batched overworld terrain and debug grid geometry submission for the render shell.

## Requirements

### Requirement: Render shell batches visible terrain geometry
The game render shell SHALL translate the visible terrain entries from the active render snapshot into shared textured geometry and submit that geometry without issuing one SFML terrain draw per visible tile.

#### Scenario: Visible terrain is submitted as a batched layer
- **WHEN** the game renders an overworld frame containing visible terrain tiles
- **THEN** the render shell builds one shared terrain geometry layer from the visible terrain entries for that frame
- **AND** it submits that terrain layer without iterating visible tiles into separate terrain draw calls

#### Scenario: Batched terrain follows the active presentation camera
- **WHEN** the active viewport size or debug zoom level changes
- **THEN** the render shell rebuilds the visible terrain geometry from the current render snapshot
- **AND** the submitted terrain layer remains aligned to the same render-facing camera frame used for visibility collection

### Requirement: Render shell batches debug tile-grid geometry
The game render shell SHALL translate the optional debug tile-grid overlay into shared colored geometry and submit that geometry without issuing one SFML rectangle draw for each tile-edge segment.

#### Scenario: Grid overlay is submitted as a batched layer
- **WHEN** debug view enables the tile-grid overlay for an overworld frame
- **THEN** the render shell builds one shared grid geometry layer from the visible terrain tiles for that frame
- **AND** it submits that grid layer without iterating visible tile edges into separate rectangle draw calls

#### Scenario: Grid overlay remains frame-local
- **WHEN** debug view disables the tile-grid overlay
- **THEN** the render shell does not submit the batched grid geometry layer for subsequent frames
- **AND** terrain rendering continues without leaving stale grid geometry on screen
