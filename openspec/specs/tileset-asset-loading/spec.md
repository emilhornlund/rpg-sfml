## Purpose

Define the runtime boundary for loading wrapped tileset classification documents and resolving their referenced metadata from staged assets.

## Requirements

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

### Requirement: Vegetation classification loading preserves placement metadata
The runtime SHALL preserve vegetation anchor placement metadata from the staged vegetation classification, including `placeOn` tile constraints, `biomes` weights, and `placementMode`, so vegetation placement can use the staged asset data as its source of truth.

#### Scenario: Water vegetation metadata survives loading
- **WHEN** runtime vegetation metadata is loaded from the staged overworld vegetation classification
- **THEN** a vegetation prototype such as a water lily retains its allowed anchor tile classes, biome weights, and `placementMode`
- **AND** later runtime placement logic can query that metadata without reopening the raw classification document

#### Scenario: Placement metadata is preserved for existing land vegetation
- **WHEN** runtime vegetation metadata is loaded for a forest or grass prototype
- **THEN** the runtime retains the prototype's allowed anchor tile classes, biome weights, and `placementMode` from the staged classification
- **AND** the loaded metadata is available alongside the prototype's geometry and atlas-part information
