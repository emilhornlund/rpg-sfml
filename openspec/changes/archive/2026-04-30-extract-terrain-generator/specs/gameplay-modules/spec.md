## ADDED Requirements

### Requirement: World generation collaborator boundary
The runtime SHALL allow the `World` module to delegate deterministic world-generation rules to dedicated world-owned collaborators so procedural terrain construction can evolve without expanding the gameplay-facing `World` boundary.

#### Scenario: Terrain generation is separated from world runtime queries
- **WHEN** the overworld implementation adds or updates terrain-generation rules
- **THEN** those rules are defined through dedicated world-generation collaborators or helpers rather than accumulating inside `World`'s gameplay-facing runtime logic
- **AND** `World` remains responsible for owning generated state and exposing world-facing queries such as dimensions, traversability, tile lookup, and spawn access
