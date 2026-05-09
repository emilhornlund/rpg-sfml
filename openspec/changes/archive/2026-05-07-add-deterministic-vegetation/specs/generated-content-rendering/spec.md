## MODIFIED Requirements

### Requirement: Generated content renders as a world layer
The outer game shell SHALL render generated world content from snapshot-published data as part of the overworld world view instead of hiding generated content behind world-only queries. Terrain SHALL remain a dedicated terrain pass, while vegetation content and player presentation SHALL be rendered through y-sorted object presentation so overlap is determined from published world-space ordering data rather than a fixed "generated content below player" rule.

#### Scenario: Vegetation and player presentation render with y-sorted overlap
- **WHEN** the shell renders an overworld frame that includes visible vegetation content and the player marker
- **THEN** it draws terrain before object presentation
- **AND** it renders vegetation content and player presentation using snapshot-published ordering data in deterministic y-sorted order
- **AND** the resulting overlap allows the player to appear behind or in front of vegetation according to world-space depth
