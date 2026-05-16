## ADDED Requirements

### Requirement: Runtime preserves overlay-classified tileset metadata from staged catalogs
The runtime SHALL preserve overlay-classified tile metadata from staged tileset catalogs, including overlay identifier, overlay class, underlying surface selector, and autotile role metadata, so road overlay rendering can resolve atlas cells without reopening raw catalog JSON.

#### Scenario: Ground overlay catalog loads overlay metadata
- **WHEN** runtime code loads the staged overworld ground overlay catalog
- **THEN** it preserves each overlay tile's identifier, overlay class, optional surface selector, and optional autotile metadata alongside atlas coordinates and wrapped tileset metadata
- **AND** later runtime helpers can resolve road overlay presentation from the loaded catalog document without reparsing the source file

