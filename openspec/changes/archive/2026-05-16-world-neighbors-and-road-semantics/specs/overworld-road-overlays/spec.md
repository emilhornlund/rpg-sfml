## MODIFIED Requirements

### Requirement: World publishes deterministic road overlay tiles
The overworld runtime SHALL derive dirt-road overlay membership from a world-owned stamped road field produced from deterministic road-network data instead of from render-time heuristics, spawn-centered band logic, or direct boolean footprint overlap. The same world seed and world-space coordinates SHALL produce the same road-covered tile coordinates across repeated frames and repeated launches. The published road shape SHALL support topology-backed branches, loops, segment endpoints, destination-connected approaches, explicit width rules, shoulders, curves, intersections, dead ends, corners, tees, crosses, and plazas while keeping the same occupancy results for world queries and world-content placement rules.

#### Scenario: Repeated road queries stay stable
- **WHEN** the runtime resolves visible road overlays for the same world seed and the same camera-bounded area multiple times
- **THEN** it publishes the same set of road-covered tile coordinates on every query
- **AND** the result does not depend on camera movement history, chunk retention order, or runtime random state

#### Scenario: Topology-backed road occupancy supports richer road layouts
- **WHEN** the runtime queries road overlay membership for tiles near the deterministic road network across repeated launches with the same seed
- **THEN** the published road occupancy can form branches, loops, destination-connected approaches, explicit width changes, readable bends, dead ends, corners, tees, crosses, plazas, and structural intersections in addition to straight segments
- **AND** those occupancy results come from the same world-owned stamped road source of truth rather than from hard-coded spawn-cross logic

#### Scenario: Road-aware content exclusion matches road visibility
- **WHEN** world-content placement evaluates whether a traversable anchor tile lies inside the stamped road footprint
- **THEN** it uses the same road occupancy decision as visible road overlay collection
- **AND** generated ground content does not appear on tiles that the runtime also publishes as visible road overlays

### Requirement: Road overlays resolve atlas cells from surface-aware neighbor masks
The game shell SHALL render each visible road overlay tile by selecting from staged road overlay metadata using the tile's underlying terrain category, neighboring stamped road structure, and deterministic interior variation. Edge and corner road tiles SHALL use the surface-specific transition role for the underlying terrain. Non-transition interior road tiles SHALL deterministically select from a `base` road-variant pool and SHALL treat `decor` road variants as sparse visual replacements that remain stable for the same world coordinates, terrain category, and world seed. The renderer SHALL keep autotile rendering as the final visual pass and SHALL compose motifs such as end caps, shoulders, branch junctions, loops, curves, dead ends, corners, tees, crosses, plazas, and destination approaches from stamped local structure instead of requiring dedicated graph-specific atlas roles. Neighbor analysis for this selection SHALL use world-backed structural neighbor context published in the render snapshot rather than reconstructing neighbors from only the currently visible road overlay subset.

#### Scenario: Surface-specific road edge transitions are selected
- **WHEN** a visible road tile lies on grass, forest, or sand and its neighboring stamped road structure matches a supported edge or corner role
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

#### Scenario: Viewport-edge road tiles keep correct motifs
- **WHEN** a visible road tile depends on stamped neighbors outside the current visible overlay subset
- **THEN** the renderer still resolves the same transition or interior role it would have resolved if those neighbors were visible
- **AND** the road motif does not change solely because the camera clipped adjacent structural tiles
