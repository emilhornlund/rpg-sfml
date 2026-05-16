## ADDED Requirements

### Requirement: World publishes deterministic road overlay tiles
The overworld runtime SHALL derive dirt-road overlay membership from world-owned deterministic data instead of from render-time heuristics or per-frame random selection. The same world seed and world-space coordinates SHALL produce the same road-covered tiles across repeated frames and repeated launches.

#### Scenario: Repeated road queries stay stable
- **WHEN** the runtime resolves visible road overlays for the same world seed and the same camera-bounded area multiple times
- **THEN** it publishes the same set of road-covered tile coordinates on every query
- **AND** the result does not depend on camera movement history, chunk retention order, or runtime random state

### Requirement: Road overlays resolve atlas cells from surface-aware neighbor masks
The game shell SHALL render each visible road overlay tile by selecting from staged road overlay metadata using the tile's underlying terrain category, neighboring road occupancy, and deterministic base variation. Edge and corner road tiles SHALL use the surface-specific transition role for the underlying terrain, and non-edge road tiles SHALL use a deterministic base road variant.

#### Scenario: Surface-specific road edge transitions are selected
- **WHEN** a visible road tile lies on grass, forest, or sand and its neighboring road occupancy matches a supported edge or corner role
- **THEN** the renderer selects the transition atlas entry for that surface and resolved role
- **AND** it does not require gameplay modules to publish atlas coordinates or texture data

#### Scenario: Interior road tiles use stable base variants
- **WHEN** a visible road tile does not require a transition edge role
- **THEN** the renderer selects a base dirt-road variant from overlay metadata using a deterministic rule derived from stable tile data and the world seed
- **AND** the same road tile keeps the same base appearance across repeated frames and repeated launches

### Requirement: Road overlays render as a ground-detail pass between terrain and sorted content
The game shell SHALL render visible road overlays as a dedicated batched ground-detail layer above terrain and below generated content and gameplay markers.

#### Scenario: Road overlays appear beneath vegetation and player markers
- **WHEN** an overworld frame contains both visible road overlays and renderable generated content or player markers
- **THEN** the road overlay layer is drawn after terrain
- **AND** the road overlay layer is drawn before vegetation content and player markers

