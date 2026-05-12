## MODIFIED Requirements

### Requirement: Debug snapshot describes overlay-facing runtime values
The overworld runtime SHALL publish debug snapshot data containing the current player tile coordinates, active zoom percentage, retained chunk count, retained generated-content count for loaded chunks, visible tile count for the active frame, and visible generated-content count for the active frame.

#### Scenario: Overlay reads gameplay-derived diagnostics
- **WHEN** the outer game shell prepares to draw the debug overlay for the current frame
- **THEN** it can read player tile coordinates, zoom percentage, retained chunk count, retained generated-content count, visible tile count, and visible generated-content count from repo-native runtime snapshot data
- **AND** it does not need to inspect gameplay-owned modules directly to derive those values

#### Scenario: Snapshot metrics reflect bounded chunk retention
- **WHEN** the overworld runtime advances through exploration that causes old chunks to unload
- **THEN** the published retained chunk count and retained generated-content count describe only the chunks currently retained in the world cache
- **AND** those counts are allowed to decrease after retention pruning removes out-of-window chunks

#### Scenario: Snapshot visibility metrics reflect the active camera frame
- **WHEN** the overworld runtime prepares the current frame for rendering
- **THEN** the published visible tile count matches the number of visible tiles in the active render snapshot
- **AND** the published visible generated-content count matches the number of visible generated-content entries in the active render snapshot
