## ADDED Requirements

### Requirement: Thin game shell for overworld runtime
The game runtime SHALL keep `Game` as a thin executable shell around a dedicated overworld runtime collaborator instead of directly owning overworld session initialization, gameplay update sequencing, and render-state assembly inside `Game.cpp`.

#### Scenario: Delegating overworld session lifecycle
- **WHEN** the executable initializes and advances the active overworld gameplay loop
- **THEN** `Game` delegates overworld session initialization and per-frame overworld progression to a dedicated collaborator defined outside `Game.cpp`
- **AND** `Game` does not directly own the `World`, `Player`, and `Camera` collaboration rules for that session

#### Scenario: Preserving SFML shell responsibilities
- **WHEN** the active game loop processes a frame
- **THEN** `Game` remains responsible for SFML-facing concerns such as window lifecycle, event polling, and backend drawing
- **AND** gameplay-facing overworld orchestration remains behind the dedicated overworld runtime boundary
