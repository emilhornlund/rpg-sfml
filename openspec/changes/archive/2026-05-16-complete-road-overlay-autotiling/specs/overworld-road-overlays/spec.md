## MODIFIED Requirements

### Requirement: World publishes deterministic road overlay tiles
The overworld runtime SHALL derive dirt-road overlay membership from world-owned deterministic data instead of from render-time heuristics or per-frame random selection. The same world seed and world-space coordinates SHALL produce the same road-covered tile coordinates across repeated frames and repeated launches. The published road shape SHALL render roads at least two tiles wide, allow deterministic three-tile widening segments, and keep the same occupancy results for world queries and world-content placement rules.

#### Scenario: Repeated road queries stay stable
- **WHEN** the runtime resolves visible road overlays for the same world seed and the same camera-bounded area multiple times
- **THEN** it publishes the same set of road-covered tile coordinates on every query
- **AND** the result does not depend on camera movement history, chunk retention order, or runtime random state

#### Scenario: Widened road occupancy remains deterministic
- **WHEN** the runtime queries road overlay membership for tiles near the spawn-centered road network across repeated launches with the same seed
- **THEN** every road arm resolves to a deterministic occupancy band that is at least two tiles wide
- **AND** any local widening to three tiles occurs at the same coordinates on every launch for that seed

#### Scenario: Road-aware content exclusion matches road visibility
- **WHEN** world-content placement evaluates whether a traversable anchor tile lies inside the deterministic road footprint
- **THEN** it uses the same road occupancy decision as visible road overlay collection
- **AND** generated ground content does not appear on tiles that the runtime also publishes as visible road overlays

### Requirement: Road overlays resolve atlas cells from surface-aware neighbor masks
The game shell SHALL render each visible road overlay tile by selecting from staged road overlay metadata using the tile's underlying terrain category, neighboring road occupancy, and deterministic interior variation. Edge and corner road tiles SHALL use the surface-specific transition role for the underlying terrain. Non-transition interior road tiles SHALL deterministically select from a `base` road-variant pool and SHALL treat `decor` road variants as sparse visual replacements that remain stable for the same world coordinates, terrain category, and world seed. The renderer SHALL support composed motifs such as road end caps and widened shoulders by resolving neighboring transition roles from the widened occupancy pattern instead of requiring dedicated single-tile cap assets.

#### Scenario: Surface-specific road edge transitions are selected
- **WHEN** a visible road tile lies on grass, forest, or sand and its neighboring road occupancy matches a supported edge or corner role
- **THEN** the renderer selects the transition atlas entry for that surface and resolved role
- **AND** it does not require gameplay modules to publish atlas coordinates or texture data

#### Scenario: Interior road tiles use stable base variants
- **WHEN** a visible road tile does not require a transition edge role and the deterministic decor roll does not select a decor replacement
- **THEN** the renderer selects a base dirt-road variant from overlay metadata using a deterministic rule derived from stable tile data and the world seed
- **AND** the same road tile keeps the same base appearance across repeated frames and repeated launches

#### Scenario: Decor participates as a sparse interior replacement
- **WHEN** a visible road tile does not require a transition role and the staged road overlay metadata includes `decor` variants
- **THEN** the renderer may select a decor road variant as a low-frequency replacement for the interior base tile
- **AND** the same road tile keeps the same base-or-decor choice across repeated frames and repeated launches

#### Scenario: Widened occupancy composes road end caps without dedicated cap assets
- **WHEN** a deterministic road endpoint or local widening produces an occupancy pattern whose neighboring tiles resolve to coordinated edge and corner roles
- **THEN** the renderer composes the visible road motif from those per-tile transition selections
- **AND** the result does not require a dedicated single-tile end-cap role in the staged overlay metadata
