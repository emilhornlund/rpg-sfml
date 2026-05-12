## ADDED Requirements

### Requirement: Occlusion work is qualified by actual player overlap
The render shell SHALL narrow player-occlusion work from the broad set of front generated-content entries in the y-sorted render queue to the subset whose published world-space presentation bounds intersect the player's published world-space presentation bounds for the active frame.

#### Scenario: Front content without overlap does not become an occlusion candidate
- **WHEN** one or more generated-content entries sort in front of the player for the active frame but none of their published bounds intersect the player's published presentation bounds
- **THEN** the shell reports those entries as front occluders for diagnostics
- **AND** it does not include them in the overlap-qualified occlusion candidate set for the silhouette pass

#### Scenario: Overlapping front content becomes an occlusion candidate
- **WHEN** a generated-content entry sorts in front of the player and its published world-space presentation bounds intersect the player's published presentation bounds
- **THEN** the shell includes that entry in the overlap-qualified occlusion candidate set for the active frame
- **AND** the shell may use that qualified set to drive later occlusion mask rendering

### Requirement: Occlusion composition may use a reduced-resolution working surface
The render shell SHALL be allowed to build the player-occlusion mask and occluder mask through a reduced-resolution working surface derived from the active window size, provided the resulting composite still reveals only the occluded portion of the player silhouette and remains aligned with the player's on-screen presentation.

#### Scenario: Reduced-resolution masks preserve silhouette-only output
- **WHEN** the shell composites the player-occlusion silhouette through a reduced-resolution working surface
- **THEN** the normal player sprite remains visible where it is not occluded
- **AND** the silhouette overlay appears only where overlap-qualified occlusion candidates hide the player

#### Scenario: Reduced-resolution masks remain resize-safe
- **WHEN** the player resizes the application window and a later frame still contains overlap-qualified occlusion candidates
- **THEN** the shell derives the reduced-resolution working surface from the resized window dimensions
- **AND** the silhouette composite remains registered to the same on-screen center as the player's normal sprite
