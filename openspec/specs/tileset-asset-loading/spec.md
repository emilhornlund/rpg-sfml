## Purpose

Define the runtime boundary for loading wrapped tileset catalog documents and resolving their referenced metadata from staged assets.

## Requirements

### Requirement: Runtime loads wrapped tileset catalog documents from the staged asset root
The runtime SHALL load wrapped tileset catalog documents from the staged executable asset root using the schema emitted by the asset toolchain, including `schemaVersion`, `tileset`, and `tiles`, instead of assuming a legacy flat file in the top-level asset directory.

#### Scenario: Generated terrain catalog is loaded from staged assets
- **WHEN** the runtime or an asset-dependent test requests the overworld terrain tileset document
- **THEN** it resolves the staged catalog file from `assets/output/catalogs/overworld-terrain-tileset-catalog.json`
- **AND** it parses the wrapped document successfully without depending on legacy top-level tile metadata fields

#### Scenario: Additional staged catalogs use the same loading boundary
- **WHEN** the runtime adds another known tileset document such as the overworld vegetation tileset
- **THEN** it loads that catalog through the same tileset asset loading boundary
- **AND** it does not require a separate one-off file loader architecture

### Requirement: Runtime resolves tileset metadata relative to an asset root
The runtime SHALL resolve wrapped tileset catalog metadata relative to a known runtime asset root so generated catalog documents can live in `assets/output/catalogs/` while still referring to both authored source metadata and staged runtime atlas images. When runtime code needs a tileset texture, it SHALL resolve that texture from the catalog's explicit runtime image metadata rather than from `tileset.source.image`.

#### Scenario: Catalog runtime image resolves from wrapped metadata
- **WHEN** a loaded tileset catalog document references its runtime tileset image through `tileset.runtime.image`
- **THEN** the runtime resolves that image from the staged asset root
- **AND** the resolution remains valid when the catalog document is stored under `assets/output/catalogs/`

#### Scenario: Authored source image metadata remains non-runtime traceability data
- **WHEN** a loaded tileset catalog document includes `tileset.source.image`
- **THEN** the runtime preserves that value as authored source metadata
- **AND** runtime texture loading does not treat that source image path as the staged executable atlas location

### Requirement: Vegetation catalog loading preserves placement metadata
The runtime SHALL preserve vegetation anchor placement metadata from the staged vegetation catalog, including `placeOn` tile constraints, `biomes` weights, and `placementMode`, so vegetation placement can use the staged asset data as its source of truth.

#### Scenario: Water vegetation metadata survives loading
- **WHEN** runtime vegetation metadata is loaded from the staged overworld vegetation catalog
- **THEN** a vegetation prototype such as a water lily retains its allowed anchor tile classes, biome weights, and `placementMode`
- **AND** later runtime placement logic can query that metadata without reopening the raw catalog document

#### Scenario: Placement metadata is preserved for existing land vegetation
- **WHEN** runtime vegetation metadata is loaded for a forest or grass prototype
- **THEN** the runtime retains the prototype's allowed anchor tile classes, biome weights, and `placementMode` from the staged vegetation catalog
- **AND** the loaded metadata is available alongside the prototype's geometry and atlas-part information

### Requirement: Runtime preserves overlay-classified tileset metadata from staged catalogs
The runtime SHALL preserve overlay-classified tile metadata from staged tileset catalogs, including overlay identifier, overlay class, underlying surface selector, and autotile role metadata, so road overlay rendering can resolve atlas cells without reopening raw catalog JSON.

#### Scenario: Ground overlay catalog loads overlay metadata
- **WHEN** runtime code loads the staged overworld ground overlay catalog
- **THEN** it preserves each overlay tile's identifier, overlay class, optional surface selector, and optional autotile metadata alongside atlas coordinates and wrapped tileset metadata
- **AND** later runtime helpers can resolve road overlay presentation from the loaded catalog document without reparsing the source file
