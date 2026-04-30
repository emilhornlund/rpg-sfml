## Purpose

Define the structural boundaries and ownership rules for the core gameplay module types (World, Player, Camera) within the runtime.

## Requirements

### Requirement: Dedicated gameplay module types
The runtime SHALL define `World`, `Player`, and `Camera` as dedicated project types in their own runtime-facing files instead of declaring them inline inside the game loop coordinator implementation.

#### Scenario: Module type extraction
- **WHEN** the runtime source is organized for gameplay collaboration
- **THEN** `World`, `Player`, and `Camera` are declared through dedicated project files
- **AND** `Game.cpp` uses those types without defining them locally

### Requirement: Gameplay module ownership boundary
The runtime SHALL let each gameplay module own its own internal state so future world, player, and camera logic can be added to those modules without moving that logic into `Game`.

#### Scenario: Future gameplay growth
- **WHEN** new gameplay behavior is added for world, player, or camera concerns
- **THEN** that behavior can be implemented within the corresponding module type
- **AND** `Game` remains responsible for orchestration rather than absorbing the module's internal rules

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
