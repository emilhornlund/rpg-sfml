## MODIFIED Requirements

### Requirement: Build stages bundled runtime assets from the repository root
The project SHALL source bundled runtime assets from the repository-root `assets/` directory and SHALL stage the executable's required runtime files into the built executable's `assets/` directory using the same consolidated runtime layout under `output/`. The staged runtime files SHALL include runtime tileset PNGs under `output/tilesets/`, generated catalog files under `output/catalogs/`, runtime spritesheets under `output/spritesheets/`, and the bundled font asset required for the debug overlay text under `output/fonts/`.

#### Scenario: Main executable stages required runtime assets
- **WHEN** the main executable is built
- **THEN** the build copies `assets/output/tilesets/overworld-terrain-tileset.png`, `assets/output/catalogs/overworld-terrain-tileset-catalog.json`, `assets/output/tilesets/overworld-vegetation-tileset.png`, `assets/output/catalogs/overworld-vegetation-tileset-catalog.json`, `assets/output/spritesheets/player-walking-spritesheet.png`, and one bundled debug-overlay font from the repository-root asset tree
- **AND** those files are present in the built executable's `assets/output/` directory tree for runtime loading

### Requirement: Asset-dependent project checks use the relocated source layout
The project SHALL reference checked-in runtime asset sources from the repository-root `assets/` directory in asset-dependent checks while continuing to validate the staged runtime copies from the build output. Source-backed checks SHALL resolve runtime tilesets, catalogs, spritesheets, and fonts from their `assets/output/` locations rather than legacy root-level runtime files.

#### Scenario: Source-backed asset checks use the root asset directory
- **WHEN** an asset-dependent test or configuration check needs a checked-in runtime asset file
- **THEN** it resolves that source file from the repository-root `assets/` directory, including runtime tilesets under `assets/output/tilesets/`, generated catalogs under `assets/output/catalogs/`, runtime spritesheets under `assets/output/spritesheets/`, and fonts under `assets/output/fonts/`
- **AND** it does not depend on legacy top-level runtime asset files

#### Scenario: Staging checks keep validating executable-local build assets
- **WHEN** an asset-staging check verifies runtime availability after a build
- **THEN** it asserts the expected files under the built executable's `assets/output/` directory tree
- **AND** the check remains independent from the repository source asset path beyond the staging inputs
