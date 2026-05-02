## ADDED Requirements

### Requirement: Overworld terrain rendering selects autotile transitions from neighboring terrain
The game runtime SHALL resolve terrain presentation for each visible overworld tile from the tile's terrain category, neighboring terrain categories, and the updated tileset classification metadata instead of from a fixed one-cell-per-`TileType` mapping. The renderer SHALL normalize terrain borders into the canonical directed pairs supported by the atlas and SHALL resolve one autotile role per rendered tile for those transitions.

#### Scenario: Canonical terrain pair selection
- **WHEN** a visible terrain tile borders a neighboring terrain category that forms one of the supported transitions
- **THEN** the renderer selects the canonical directed pair defined by the tileset metadata for that terrain relationship
- **AND** it does not require mirrored atlas entries for the inverse pair direction

#### Scenario: Mixed neighboring terrain uses deterministic priority
- **WHEN** a visible terrain tile borders multiple different transition targets in the same local neighborhood
- **THEN** the renderer resolves the rendered transition using a deterministic terrain priority order
- **AND** the same terrain layout produces the same selected transition on every frame

#### Scenario: Supported neighbor mask resolves to a named autotile role
- **WHEN** a visible terrain tile matches one of the supported autotile neighborhood patterns for its selected transition pair
- **THEN** the renderer resolves exactly one role from the supported autotile role set
- **AND** it uses the atlas entry for that directed pair and role when drawing the tile

### Requirement: Overworld terrain rendering applies deterministic base variation
The game runtime SHALL select base terrain variants for non-transition grass, sand, and forest tiles from the updated tileset classification metadata through a deterministic rule derived from stable tile data. The renderer SHALL keep the selected base variant stable for the same world coordinates and terrain category across repeated frames and repeated launches with the same world configuration.

#### Scenario: Stable base terrain variation
- **WHEN** the game renders the same non-transition grass, sand, or forest tile multiple times for the same world layout
- **THEN** it selects the same classified base variant for that tile on every frame
- **AND** camera movement or chunk retention order does not change the selected variant

#### Scenario: Neighbor-driven transitions override base variation
- **WHEN** a visible terrain tile qualifies for a classified transition role to a neighboring terrain category
- **THEN** the renderer draws the transition tile for that role instead of a base terrain variant
- **AND** the tile still remains owned by its original gameplay-facing terrain category

### Requirement: Overworld terrain rendering animates water transition tiles
The game runtime SHALL support animated terrain rendering for transition pairs that target water by selecting from the classified animation frames for the resolved pair and autotile role. Water-adjacent transitions SHALL keep their pair and role selection deterministic while allowing the animation frame to advance over time.

#### Scenario: Water transition frames cycle without changing the resolved role
- **WHEN** the renderer draws a visible terrain tile whose selected transition pair targets water
- **THEN** it selects the classified animation frame for that pair and role from the active animation phase
- **AND** the same tile keeps the same directed pair and autotile role until its neighboring terrain changes

#### Scenario: Non-water transitions remain static
- **WHEN** the renderer draws a visible terrain tile whose selected transition pair does not target water
- **THEN** it uses a single classified atlas entry for the resolved role
- **AND** the rendered transition does not cycle through animation frames
