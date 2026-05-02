## MODIFIED Requirements

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
