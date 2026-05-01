## MODIFIED Requirements

### Requirement: Player animation reflects movement state
The player presentation system SHALL expose an idle frame when the player is not moving and SHALL cycle through the walking frames while the player is moving. Walking-frame selection SHALL follow a stable time-based walking cadence rather than tile-step progress so the animation keeps its intended feel even when movement speed changes.

#### Scenario: Idle player uses the standing frame
- **WHEN** the player has no movement intent for the current update
- **THEN** the published player presentation uses the idle frame for the active facing direction

#### Scenario: Moving player uses walk frames
- **WHEN** the player is moving during an overworld tile step
- **THEN** the published player presentation advances through that direction's walking frames according to the configured walking cadence over time
- **AND** the active facing direction remains consistent with the movement direction being presented
