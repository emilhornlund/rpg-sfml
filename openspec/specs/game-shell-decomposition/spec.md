## Purpose

Define the shell-level decomposition boundaries and sequencing rules for extracting `Game.cpp` runtime concerns into dedicated support collaborators.

## Requirements

### Requirement: Game shell delegates extracted runtime concerns through dedicated support collaborators
The executable runtime SHALL preserve `Game` as the top-level SFML shell while moving shell-owned runtime concerns out of `src/main/Game.cpp` into dedicated support collaborators or helper modules with cohesive ownership boundaries.

#### Scenario: Game shell remains the coordinator after decomposition
- **WHEN** the runtime processes an overworld frame after the decomposition change
- **THEN** `Game` continues to own the top-level frame lifecycle and `Game::Impl`
- **AND** shell-owned concerns such as overlay handling, resource bootstrap, event translation, render-plan assembly, occlusion compositing, or world-pass rendering may be delegated to support collaborators instead of being implemented inline in `Game.cpp`

### Requirement: Debug overlay extraction is the first decomposition slice
The decomposition plan SHALL begin by extracting the debug overlay pipeline before extracting the player-occlusion compositor or the full overworld scene renderer.

#### Scenario: First runtime-shell extraction targets the overlay pipeline
- **WHEN** implementation starts on the `Game.cpp` decomposition change
- **THEN** the first extraction ticket moves the debug overlay pipeline out of `Game.cpp`
- **AND** later tickets may extract resource bootstrap, event translation, render-plan assembly, player-occlusion compositing, and world-pass rendering in sequence

### Requirement: Render-shell extraction sequencing accounts for current boundary-test constraints
The decomposition SHALL update render-related boundary coverage before moving render-plan assembly or player-occlusion compositing out of `Game.cpp` when existing tests would otherwise pin those concerns to inline implementation.

#### Scenario: Boundary checks are updated before deeper render extraction
- **WHEN** the decomposition reaches render-plan assembly or player-occlusion compositing
- **THEN** the change updates the relevant boundary checks so helper-owned logic is not required to remain inline in `Game.cpp`
- **AND** the runtime still retains test coverage for the responsibilities that must remain owned by the top-level game shell
