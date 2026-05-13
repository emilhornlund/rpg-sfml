## MODIFIED Requirements

### Requirement: Runtime loads wrapped tileset classification documents from the staged asset root
The runtime SHALL load wrapped tileset catalog documents from the staged executable asset root using the schema emitted by the asset toolchain, including `schemaVersion`, `tileset`, and `tiles`, instead of assuming a legacy flat file in the top-level asset directory.

#### Scenario: Generated terrain classification is loaded from staged assets
- **WHEN** the runtime or an asset-dependent test requests the overworld terrain tileset document
- **THEN** it resolves the staged catalog file from `assets/output/catalogs/overworld-terrain-tileset-catalog.json`
- **AND** it parses the wrapped document successfully without depending on legacy top-level tile metadata fields

#### Scenario: Additional staged classifications use the same loading boundary
- **WHEN** the runtime adds another known tileset document such as the overworld vegetation tileset
- **THEN** it loads that catalog through the same tileset asset loading boundary
- **AND** it does not require a separate one-off file loader architecture

### Requirement: Runtime resolves tileset metadata relative to an asset root
The runtime SHALL resolve tileset metadata, including the referenced image path and grid information, relative to a known runtime asset root so wrapped catalog documents can live in generated subdirectories while still referring to staged tileset images in the asset root.

#### Scenario: Classification image source resolves from wrapped metadata
- **WHEN** a loaded tileset catalog document references its tileset image through `tileset.source.image`
- **THEN** the runtime resolves that image from the staged asset root
- **AND** the resolution remains valid even when the catalog document is stored under `assets/output/catalogs/`

### Requirement: Vegetation classification loading preserves placement metadata
The runtime SHALL preserve vegetation anchor placement metadata from the staged vegetation catalog, including `placeOn` tile constraints, `biomes` weights, and `placementMode`, so vegetation placement can use the staged asset data as its source of truth.

#### Scenario: Water vegetation metadata survives loading
- **WHEN** runtime vegetation metadata is loaded from the staged overworld vegetation catalog
- **THEN** a vegetation prototype such as a water lily retains its allowed anchor tile classes, biome weights, and `placementMode`
- **AND** later runtime placement logic can query that metadata without reopening the raw catalog document

#### Scenario: Placement metadata is preserved for existing land vegetation
- **WHEN** runtime vegetation metadata is loaded for a forest or grass prototype
- **THEN** the runtime retains the prototype's allowed anchor tile classes, biome weights, and `placementMode` from the staged vegetation catalog
- **AND** the loaded metadata is available alongside the prototype's geometry and atlas-part information
