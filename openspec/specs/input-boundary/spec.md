# Input Boundary Capability

## Purpose

Establishes a clean input translation boundary between the SFML shell and gameplay-facing modules. Input polling stays at the shell edge, and gameplay consumes repo-native input types that can be tested independently of live SFML behavior.

## Requirements

### Requirement: Runtime input translation stays at the shell boundary
The executable runtime SHALL keep backend-specific input polling and key inspection at the SFML shell boundary instead of performing that polling inside gameplay-facing overworld coordination or gameplay module code.

#### Scenario: Polling input for an overworld frame
- **WHEN** the game runtime prepares input for the current overworld frame
- **THEN** it polls backend-specific key state from the SFML shell boundary
- **AND** gameplay-facing runtime collaborators do not directly inspect SFML key state

### Requirement: Debug overworld view controls stay at the shell boundary
The runtime SHALL keep debug-only overworld view control polling and key inspection at the SFML shell boundary instead of performing that polling inside gameplay-facing overworld coordination or gameplay module code. The translated debug controls SHALL remain repo-native when they are passed beyond the shell boundary, and shell-owned debug overlay visibility controls SHALL also be handled at that boundary without exposing SFML input types beyond it.

#### Scenario: Applying debug zoom input for an overworld frame
- **WHEN** the game runtime receives a debug-only zoom hotkey press during development or testing
- **THEN** the SFML shell translates that key press into repo-native debug view control state for the current overworld frame
- **AND** gameplay-facing runtime collaborators do not directly inspect SFML key codes to determine zoom behavior

#### Scenario: Toggling the tile-grid overlay for an overworld frame
- **WHEN** the game runtime receives the debug-only tile-grid toggle hotkey during development or testing
- **THEN** the SFML shell translates that key press into repo-native debug overlay state
- **AND** the resulting state can be consumed without exposing SFML input types beyond the shell boundary

#### Scenario: Toggling the debug information overlay
- **WHEN** the game runtime receives the `F1` debug-overlay hotkey during development or testing
- **THEN** the SFML shell toggles overlay visibility at the shell boundary
- **AND** gameplay-facing runtime collaborators do not directly inspect SFML key codes to determine overlay visibility

### Requirement: Overworld gameplay consumes repo-native input
The overworld gameplay update path SHALL consume repo-native per-frame input represented through project-defined types instead of SFML input types or raw backend key codes. That input SHALL publish at most one active cardinal movement direction for the frame, and simultaneous perpendicular directional keys SHALL resolve to a single direction using the shell boundary's last-pressed-wins policy.

#### Scenario: Advancing the overworld from translated input
- **WHEN** the executable runtime updates the active overworld frame
- **THEN** it passes repo-native overworld input into gameplay coordination
- **AND** that input carries the movement state needed for the current frame without exposing SFML types

#### Scenario: Resolving simultaneous perpendicular movement keys
- **WHEN** the player holds two perpendicular movement keys during an overworld frame
- **THEN** the translated repo-native input exposes only one active cardinal movement direction
- **AND** the chosen direction matches the most recently pressed held direction

### Requirement: Input translation is testable outside the live shell loop
The runtime SHALL structure overworld input translation so its repo-native results can be verified through focused automated tests without requiring gameplay tests to depend on live SFML polling behavior.

#### Scenario: Verifying translated movement input
- **WHEN** automated tests exercise the overworld input translation path
- **THEN** they can assert the resulting repo-native input values for representative directional key combinations
- **AND** those checks include canceled opposite directions and perpendicular last-pressed-wins combinations
