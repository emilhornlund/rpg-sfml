## MODIFIED Requirements

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
