## MODIFIED Requirements

### Requirement: Generated content renders as a world layer
The outer game shell SHALL render generated world content from snapshot-published data as part of the overworld world view instead of hiding generated content behind world-only queries. Terrain SHALL remain a dedicated terrain pass, while vegetation content and player presentation SHALL continue to render through one deterministic y-sorted object presentation order so overlap is determined from published world-space ordering data rather than a fixed "generated content below player" rule. When front objects in that y-sorted order occlude the player, the shell SHALL reveal only the occluded portion of the player as a silhouette overlay while leaving the visible portion of the player sprite unchanged.

#### Scenario: Vegetation and player presentation render with y-sorted overlap
- **WHEN** the shell renders an overworld frame that includes visible vegetation content and the player marker
- **THEN** it draws terrain before object presentation
- **AND** it renders vegetation content and player presentation using snapshot-published ordering data in deterministic y-sorted order
- **AND** the resulting overlap allows the player to appear behind or in front of vegetation according to world-space depth

#### Scenario: Occluded portion of player remains readable
- **WHEN** one or more generated-content entries render in front of the player in the y-sorted object order
- **THEN** the shell keeps the normal player sprite visible where it is not occluded
- **AND** it draws a silhouette overlay only where the player is actually hidden by those front objects
- **AND** the silhouette effect does not require fading the entire occluding object

#### Scenario: Unoccluded player remains unchanged
- **WHEN** no generated-content entry in the current y-sorted object order occludes the player
- **THEN** the shell does not draw an occlusion silhouette for that frame
- **AND** the player's normal sprite presentation remains unchanged
