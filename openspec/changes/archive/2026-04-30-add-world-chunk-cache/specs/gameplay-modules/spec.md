## MODIFIED Requirements

### Requirement: World generation collaborator boundary
The runtime SHALL allow the `World` module to delegate deterministic world-generation rules to dedicated world-owned collaborators so procedural terrain construction, chunk coordinate handling, and chunk cache behavior can evolve without expanding the gameplay-facing `World` boundary.

#### Scenario: Terrain generation is separated from world runtime queries
- **WHEN** the overworld implementation adds or updates terrain-generation rules
- **THEN** those rules are defined through dedicated world-generation collaborators or helpers rather than accumulating inside `World`'s gameplay-facing runtime logic
- **AND** `World` remains responsible for owning generated state and exposing world-facing queries such as dimensions, traversability, tile lookup, and spawn access

#### Scenario: Chunk generation stays behind the world boundary
- **WHEN** the runtime adds chunk-addressed generation or chunk caching for overworld terrain
- **THEN** `Game`, `Player`, and `Camera` continue to interact with world data through `World`
- **AND** chunk coordinate math and cache ownership remain encapsulated in world-owned implementation details or collaborators
