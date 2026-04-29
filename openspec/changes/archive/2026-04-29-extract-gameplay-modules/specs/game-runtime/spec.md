## MODIFIED Requirements

### Requirement: Extensible gameplay coordination
The runtime SHALL coordinate gameplay-facing systems through dedicated domain objects defined outside the top-level loop coordinator so that future world, player, and camera logic can be integrated without moving their rules or type definitions into `Game.cpp`.

#### Scenario: Runtime integration boundary
- **WHEN** gameplay systems are introduced for world, player, or camera behavior
- **THEN** the runtime invokes those systems through owned objects or collaborators defined in dedicated project files
- **AND** the top-level loop coordinator does not declare those gameplay types inline
