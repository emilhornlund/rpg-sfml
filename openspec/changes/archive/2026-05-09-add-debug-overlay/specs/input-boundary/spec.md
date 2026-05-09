## MODIFIED Requirements

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
