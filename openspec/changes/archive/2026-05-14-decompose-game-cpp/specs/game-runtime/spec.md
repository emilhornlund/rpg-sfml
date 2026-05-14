## MODIFIED Requirements

### Requirement: Thin game shell for overworld runtime
The game runtime SHALL keep `Game` as a thin executable shell around a dedicated overworld runtime collaborator and dedicated shell-support collaborators instead of directly owning overworld session initialization, gameplay update sequencing, render-state assembly, debug overlay drawing, occlusion compositing, and other shell-owned concerns inline inside `Game.cpp`.

#### Scenario: Delegating overworld session lifecycle
- **WHEN** the executable initializes and advances the active overworld gameplay loop
- **THEN** `Game` delegates overworld session initialization and per-frame overworld progression to a dedicated collaborator defined outside `Game.cpp`
- **AND** `Game` does not directly own the `World`, `Player`, and `Camera` collaboration rules for that session

#### Scenario: Rendering from gameplay snapshots
- **WHEN** the active game loop renders an overworld frame
- **THEN** `Game` consumes a render snapshot supplied by the dedicated overworld runtime boundary
- **AND** shell-owned rendering concerns may be routed through dedicated support collaborators instead of remaining inline in `Game.cpp`
- **AND** `Game` does not assemble overworld presentation state ad hoc from gameplay internals during drawing

#### Scenario: Preserving SFML shell responsibilities
- **WHEN** the active game loop processes a frame
- **THEN** `Game` remains responsible for SFML-facing concerns such as window lifecycle, event polling, and backend drawing coordination
- **AND** gameplay-facing overworld orchestration and snapshot assembly remain behind the dedicated overworld runtime boundary
- **AND** shell-owned overlay, render-planning, and compositing concerns may be delegated to dedicated support collaborators without changing the public `Game` entry point
