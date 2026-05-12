## ADDED Requirements

### Requirement: Tile-grid batching avoids duplicated interior edge geometry
The game runtime SHALL build the debug tile-grid overlay from the rectangular bounds implied by the visible tile set instead of expanding each visible tile into four independent edge rectangles.

#### Scenario: Grid overlay emits one strip per visible boundary
- **WHEN** debug view enables the tile-grid overlay for an overworld frame with `W` visible tile columns and `H` visible tile rows
- **THEN** the render shell builds `W + 1` vertical strips and `H + 1` horizontal strips for that frame
- **AND** it does not rebuild every visible tile edge as a separate rectangle

#### Scenario: Grid overlay preserves current world-space appearance
- **WHEN** the optimized tile-grid overlay is visible during movement, resize, or debug zoom changes
- **THEN** the grid remains aligned to the active visible tile boundaries in world space
- **AND** the outer border keeps the configured line thickness while interior boundaries remain visually equivalent to the current doubled-edge presentation

#### Scenario: Grid overlay metrics reflect optimized geometry
- **WHEN** the debug overlay is visible while the tile-grid overlay is enabled
- **THEN** the reported tile-grid vertex count reflects the bounds-derived strip geometry for the active frame
- **AND** disabling the tile-grid overlay resets the reported tile-grid vertex count to zero for subsequent frames
