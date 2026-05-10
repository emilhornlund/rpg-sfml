## ADDED Requirements

### Requirement: Camera frame sizing matches the active window dimensions
The overworld runtime SHALL publish render snapshots whose camera-frame size is derived from the active runtime viewport dimensions for the current frame so that overworld rendering stays aligned with the current window size.

#### Scenario: Snapshot camera frame reflects resized viewport
- **WHEN** the active window dimensions change before an overworld frame is updated
- **THEN** the published render snapshot uses a camera-frame size calculated from the updated viewport dimensions for that frame
- **AND** the outer game shell can render the overworld without stretching content because of stale camera-frame sizing
