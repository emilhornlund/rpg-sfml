## MODIFIED Requirements

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
