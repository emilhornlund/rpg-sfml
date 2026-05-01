## ADDED Requirements

### Requirement: Runtime input translation stays at the shell boundary
The executable runtime SHALL keep backend-specific input polling and key inspection at the SFML shell boundary instead of performing that polling inside gameplay-facing overworld coordination or gameplay module code.

#### Scenario: Polling input for an overworld frame
- **WHEN** the game runtime prepares input for the current overworld frame
- **THEN** it polls backend-specific key state from the SFML shell boundary
- **AND** gameplay-facing runtime collaborators do not directly inspect SFML key state

### Requirement: Overworld gameplay consumes repo-native input
The overworld gameplay update path SHALL consume repo-native per-frame input represented through project-defined types instead of SFML input types or raw backend key codes.

#### Scenario: Advancing the overworld from translated input
- **WHEN** the executable runtime updates the active overworld frame
- **THEN** it passes repo-native overworld input into gameplay coordination
- **AND** that input carries the movement state needed for the current frame without exposing SFML types

### Requirement: Input translation is testable outside the live shell loop
The runtime SHALL structure overworld input translation so its repo-native results can be verified through focused automated tests without requiring gameplay tests to depend on live SFML polling behavior.

#### Scenario: Verifying translated movement input
- **WHEN** automated tests exercise the overworld input translation path
- **THEN** they can assert the resulting repo-native input values for representative directional key combinations
- **AND** gameplay-facing tests continue to interact with repo-native input types instead of SFML-specific polling APIs
