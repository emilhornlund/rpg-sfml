## MODIFIED Requirements

### Requirement: World generation collaborator boundary
The runtime SHALL allow the `World` module to delegate deterministic world-generation policy to dedicated world-owned collaborators so biome sampling, terrain generation, and similar generation rules can evolve without expanding the gameplay-facing `World` boundary. `World` SHALL remain responsible for owning runtime world state, retained chunk data, and world-facing queries built on that retained state.

#### Scenario: Terrain generation policy is separated from runtime world queries
- **WHEN** the overworld implementation adds or updates deterministic generation rules
- **THEN** those rules are defined through dedicated world-generation collaborators or helpers rather than accumulating inside `World`'s gameplay-facing runtime logic
- **AND** `World` remains responsible for exposing world-facing queries such as traversability, tile lookup, and spawn access from its retained runtime state

#### Scenario: Runtime chunk state stays behind the world boundary
- **WHEN** the runtime adds chunk-addressed terrain generation or other generated chunk content
- **THEN** `Game`, `Player`, and `Camera` continue to interact with world data only through `World`
- **AND** chunk cache ownership and runtime chunk reuse remain encapsulated in `World` even when chunk content is produced by dedicated generation collaborators
