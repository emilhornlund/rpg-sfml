## Purpose

Define the runtime entry point and core loop responsibilities for the game executable.

## Requirements

### Requirement: Runtime entry point
The game executable SHALL start gameplay by constructing a dedicated runtime object and transferring control of the main loop to it instead of implementing the loop directly in `main.cpp`.

#### Scenario: Startup handoff
- **WHEN** the executable starts successfully
- **THEN** the entry point constructs the runtime object and invokes its top-level run method

### Requirement: Runtime file organization
The initial runtime SHALL expose the `Game` class through `include/main/Game.hpp` and implement it in `src/main/Game.cpp`.

#### Scenario: Game runtime layout
- **WHEN** the game-loop architecture change is implemented
- **THEN** the `Game` declaration is provided from `include/main/Game.hpp` and its implementation is provided from `src/main/Game.cpp`

### Requirement: Namespaced runtime declarations
The runtime SHALL declare `Game` and related runtime types in a project-scoped namespace instead of the global namespace.

#### Scenario: Runtime symbol scope
- **WHEN** runtime types are declared for the game-loop architecture
- **THEN** those types are placed in the project namespace hierarchy rather than at global scope

### Requirement: Structured game loop phases
The runtime SHALL execute the game loop through distinct event-processing, update, and render phases.

#### Scenario: Frame lifecycle
- **WHEN** the runtime processes a frame while the window is open
- **THEN** it performs event handling, then simulation update, then rendering in that order

### Requirement: Window lifecycle ownership
The runtime SHALL own the application window and keep the loop running only while that window remains open.

#### Scenario: Window close request
- **WHEN** the user closes the window or triggers the configured exit action
- **THEN** the runtime closes the window and exits the loop cleanly

### Requirement: Public header dependency boundary
The public `Game` header SHALL avoid direct SFML includes when forward declarations and implementation indirection are sufficient to preserve the declaration boundary.

#### Scenario: Lightweight game header
- **WHEN** `include/main/Game.hpp` is introduced
- **THEN** it exposes the `Game` declaration without pulling SFML headers into the public interface when a forward-declaration strategy can satisfy the header

### Requirement: Runtime file conventions
The runtime header and source SHALL use the project file banner, the header SHALL use an include guard consistent with its file path, and the public runtime header SHALL document its public API with Doxygen comments.

#### Scenario: Game header conventions
- **WHEN** `include/main/Game.hpp` is created
- **THEN** it includes the project file banner, the `RPG_MAIN_GAME_HPP` include guard, and Doxygen comments describing the public runtime interface

### Requirement: Extensible gameplay coordination
The runtime SHALL coordinate gameplay-facing systems through dedicated domain objects defined outside the top-level loop coordinator so that future world, player, and camera logic can be integrated without moving their rules or type definitions into `Game.cpp`.

#### Scenario: Runtime integration boundary
- **WHEN** gameplay systems are introduced for world, player, or camera behavior
- **THEN** the runtime invokes those systems through owned objects or collaborators defined in dedicated project files
- **AND** the top-level loop coordinator does not declare those gameplay types inline

### Requirement: Thin game shell for overworld runtime
The game runtime SHALL keep `Game` as a thin executable shell around a dedicated overworld runtime collaborator instead of directly owning overworld session initialization, gameplay update sequencing, and render-state assembly inside `Game.cpp`.

#### Scenario: Delegating overworld session lifecycle
- **WHEN** the executable initializes and advances the active overworld gameplay loop
- **THEN** `Game` delegates overworld session initialization and per-frame overworld progression to a dedicated collaborator defined outside `Game.cpp`
- **AND** `Game` does not directly own the `World`, `Player`, and `Camera` collaboration rules for that session

#### Scenario: Rendering from gameplay snapshots
- **WHEN** the active game loop renders an overworld frame
- **THEN** `Game` consumes a render snapshot supplied by the dedicated overworld runtime boundary
- **AND** `Game` does not assemble overworld presentation state ad hoc from gameplay internals during drawing

#### Scenario: Preserving SFML shell responsibilities
- **WHEN** the active game loop processes a frame
- **THEN** `Game` remains responsible for SFML-facing concerns such as window lifecycle, event polling, and backend drawing
- **AND** gameplay-facing overworld orchestration and snapshot assembly remain behind the dedicated overworld runtime boundary

### Requirement: Runtime viewport follows the active window size
The game runtime SHALL derive the overworld viewport dimensions from the current application window size for each active frame instead of continuing to use only the startup window dimensions after the window has been resized.

#### Scenario: Resized window updates subsequent overworld frames
- **WHEN** the player resizes the application window during active gameplay
- **THEN** the runtime uses the resized window dimensions when preparing overworld input for subsequent frames
- **AND** the overworld runtime no longer receives the original startup viewport size for those frames

### Requirement: Screen-space overlay presentation follows the active window size
The game runtime SHALL maintain explicit screen-space presentation state for shell-owned overlay rendering that depends on window pixel dimensions. When the active window size changes, the runtime SHALL update that screen-space presentation state before drawing later overlay passes so resize-sensitive overlays do not continue rendering against stale screen coordinates.

#### Scenario: Resize updates later overlay composition
- **WHEN** the player resizes the application window during active gameplay
- **THEN** the runtime updates the screen-space presentation state used for later shell-owned overlay passes
- **AND** subsequent overlay drawing uses the resized window dimensions instead of the previous window size

#### Scenario: World-space and screen-space presentation remain distinct
- **WHEN** the runtime renders an overworld frame that includes both world-space scene drawing and shell-owned overlays
- **THEN** terrain, generated content, and gameplay markers continue to use the published overworld camera frame
- **AND** shell-owned overlay composition uses explicit screen-space presentation state derived from the active window size
