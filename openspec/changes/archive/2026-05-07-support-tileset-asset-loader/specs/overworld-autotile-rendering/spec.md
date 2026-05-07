## MODIFIED Requirements

### Requirement: Overworld terrain rendering selects autotile transitions from neighboring terrain
The game runtime SHALL resolve terrain presentation for each visible overworld tile from the tile's terrain category, neighboring terrain categories, and the updated tileset classification metadata instead of from a fixed one-cell-per-`TileType` mapping. The renderer SHALL normalize terrain borders into the canonical directed pairs supported by the atlas and SHALL resolve one autotile role per rendered tile for those transitions.

#### Scenario: Canonical terrain pair selection
- **WHEN** a visible terrain tile borders a neighboring terrain category that forms one of the supported transitions
- **THEN** the renderer selects the canonical directed pair defined by the wrapped terrain classification metadata for that terrain relationship
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
The game runtime SHALL select the default terrain appearance for non-transition grass, sand, and forest tiles from the updated tileset classification metadata through a deterministic rule derived from the world configuration seed, stable tile data, and the tile's terrain category. The renderer SHALL treat classified biome decor tiles as low-frequency replacements within that default appearance selection while keeping the selected result stable for the same world coordinates, terrain category, and world configuration across repeated frames and repeated launches.

#### Scenario: Stable default terrain appearance selection
- **WHEN** the game renders the same non-transition grass, sand, or forest tile multiple times for the same world layout and world configuration
- **THEN** it selects the same classified base-or-decor appearance for that tile on every frame
- **AND** camera movement or chunk retention order does not change the selected appearance

#### Scenario: Decor participates as a low-frequency biome-specific replacement
- **WHEN** the game renders a non-transition grass, sand, or forest tile whose biome has classified decor variants
- **THEN** the renderer may select a decor tile from that biome as the tile's default appearance
- **AND** the selected decor behaves as a visual replacement for the normal base tile instead of changing the tile's gameplay-facing terrain category or traversability

#### Scenario: Neighbor-driven transitions override default appearance selection
- **WHEN** a visible terrain tile qualifies for a classified transition role to a neighboring terrain category
- **THEN** the renderer draws the transition tile for that role instead of a base-or-decor default appearance
- **AND** the tile still remains owned by its original gameplay-facing terrain category
