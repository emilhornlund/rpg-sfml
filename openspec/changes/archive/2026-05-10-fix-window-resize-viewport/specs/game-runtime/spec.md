## ADDED Requirements

### Requirement: Runtime viewport follows the active window size
The game runtime SHALL derive the overworld viewport dimensions from the current application window size for each active frame instead of continuing to use only the startup window dimensions after the window has been resized.

#### Scenario: Resized window updates subsequent overworld frames
- **WHEN** the player resizes the application window during active gameplay
- **THEN** the runtime uses the resized window dimensions when preparing overworld input for subsequent frames
- **AND** the overworld runtime no longer receives the original startup viewport size for those frames
