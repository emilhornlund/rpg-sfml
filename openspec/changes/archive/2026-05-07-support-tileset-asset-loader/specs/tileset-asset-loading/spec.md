## ADDED Requirements

### Requirement: Runtime loads wrapped tileset classification documents from the staged asset root
The runtime SHALL load tileset classification documents from the staged executable asset root using the wrapped schema emitted by the asset toolchain, including `schemaVersion`, `tileset`, and `tiles`, instead of assuming a legacy flat classification file in the top-level asset directory.

#### Scenario: Generated terrain classification is loaded from staged assets
- **WHEN** the runtime or an asset-dependent test requests the overworld terrain classification
- **THEN** it resolves the staged classification file from `assets/output/classifications/overworld-terrain-tileset-classification.json`
- **AND** it parses the wrapped document successfully without depending on legacy top-level tile metadata fields

#### Scenario: Additional staged classifications use the same loading boundary
- **WHEN** the runtime adds another known classification such as the overworld vegetation tileset
- **THEN** it loads that classification through the same tileset asset loading boundary
- **AND** it does not require a separate one-off classification file loader architecture

### Requirement: Runtime resolves tileset metadata relative to an asset root
The runtime SHALL resolve tileset metadata, including the referenced image path and grid information, relative to a known runtime asset root so wrapped classification documents can live in generated subdirectories while still referring to staged tileset images in the asset root.

#### Scenario: Classification image source resolves from wrapped metadata
- **WHEN** a loaded tileset classification document references its tileset image through `tileset.source.image`
- **THEN** the runtime resolves that image from the staged asset root
- **AND** the resolution remains valid even when the classification document is stored under `assets/output/classifications/`
