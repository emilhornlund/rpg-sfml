## ADDED Requirements

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
