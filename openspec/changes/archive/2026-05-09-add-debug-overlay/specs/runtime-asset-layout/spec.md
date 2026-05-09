## MODIFIED Requirements

### Requirement: Build stages bundled runtime assets from the repository root
The project SHALL source bundled runtime assets from the repository-root `assets/` directory and SHALL stage the executable's required runtime files into the built executable's `assets/` directory, including generated classification files under `output/classifications/` when they are required for runtime loading. The staged runtime files SHALL include the bundled font asset required for the debug overlay text.

#### Scenario: Main executable stages required runtime assets
- **WHEN** the main executable is built
- **THEN** the build copies `overworld-terrain-tileset.png`, `assets/output/classifications/overworld-terrain-tileset-classification.json`, `overworld-vegetation-tileset.png`, `assets/output/classifications/overworld-vegetation-tileset-classification.json`, `player-walking-spritesheet.png`, and one bundled debug-overlay font from the repository-root asset tree
- **AND** those files are present in the built executable's `assets/` directory for runtime loading
