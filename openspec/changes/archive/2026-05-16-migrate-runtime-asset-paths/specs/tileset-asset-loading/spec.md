## MODIFIED Requirements

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
