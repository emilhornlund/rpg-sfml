## Purpose

Define the repository and runtime asset layout required for bundled executable assets.

## Requirements

### Requirement: Build stages bundled runtime assets from the repository root
The project SHALL source bundled runtime assets from the repository-root `assets/` directory and SHALL stage the executable's required runtime files into the built executable's `assets/` directory.

#### Scenario: Main executable stages required runtime assets
- **WHEN** the main executable is built
- **THEN** the build copies `overworld-terrain-tileset.png`, `overworld-terrain-tileset-classification.json`, and `player-walking-spritesheet.png` from the repository-root `assets/` directory
- **AND** those files are present in the built executable's `assets/` directory for runtime loading

### Requirement: Asset-dependent project checks use the relocated source layout
The project SHALL reference checked-in runtime asset sources from the repository-root `assets/` directory in asset-dependent checks while continuing to validate the staged runtime copies from the build output.

#### Scenario: Source-backed asset checks use the root asset directory
- **WHEN** an asset-dependent test or configuration check needs a checked-in runtime asset file
- **THEN** it resolves that source file from the repository-root `assets/` directory
- **AND** it does not depend on `/assets/` as the source asset location

#### Scenario: Staging checks keep validating executable-local build assets
- **WHEN** an asset-staging check verifies runtime availability after a build
- **THEN** it asserts the expected files under the built executable's `assets/` directory
- **AND** the check remains independent from the repository source asset path beyond the staging inputs
