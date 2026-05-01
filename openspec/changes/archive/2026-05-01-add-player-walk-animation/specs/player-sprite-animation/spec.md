## ADDED Requirements

### Requirement: Overworld player presentation uses the walking spritesheet
The runtime SHALL render the overworld player from the bundled walking spritesheet instead of a generic marker. The change SHALL interpret `player-walking-spritesheet.png` as a fixed 3-column by 4-row sheet whose rows map to down, left, right, and up facing directions.

#### Scenario: Direction rows map deterministically
- **WHEN** the overworld runtime prepares player presentation for a facing direction
- **THEN** it selects the same spritesheet row for that direction on every frame
- **AND** the row mapping remains down, left, right, up

### Requirement: Player animation reflects movement state
The player presentation system SHALL expose an idle frame when the player is not moving and SHALL cycle through the row's walking frames while the player is moving.

#### Scenario: Idle player uses the standing frame
- **WHEN** the player has no movement intent for the current update
- **THEN** the published player presentation uses the idle frame for the active facing direction

#### Scenario: Moving player uses walk frames
- **WHEN** the player is moving during overworld updates
- **THEN** the published player presentation advances through that direction's walking frames over time
- **AND** the active facing direction remains consistent with the movement direction being presented

### Requirement: Player sprite placement aligns to the tile-centered foot position
The runtime SHALL align the player sprite to the player's logical world position using a stable foot pivot so padded frame cells remain centered on the occupied 16x16 tile while the character extends upward into the tile above.

#### Scenario: Sprite remains centered on the occupied tile
- **WHEN** the player is positioned on a world tile
- **THEN** the published player sprite placement anchors the sprite pivot at the player's world position
- **AND** the resulting draw geometry keeps the character visually centered on that tile rather than centering the full padded frame cell
