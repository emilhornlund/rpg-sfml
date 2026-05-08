## Purpose

Define how deterministic overworld vegetation content is derived, loaded, and exposed for rendering.

## Requirements

### Requirement: World derives deterministic vegetation from world-space sampling
The world-owned vegetation system SHALL derive vegetation instances from deterministic world-space sampling inputs based on the world generation seed and stable world-space coordinates instead of runtime randomness or chunk-generation order.

#### Scenario: Repeated vegetation queries resolve the same world-space instances
- **WHEN** the runtime resolves vegetation for the same world seed and the same world-space area multiple times
- **THEN** it returns the same vegetation instances with the same identities, anchor positions, geometry, and appearance selectors
- **AND** the result does not depend on prior chunk generation order or runtime random state

### Requirement: Vegetation placement follows biome-specific density rules
The vegetation system SHALL apply biome-sensitive placement rules so forest terrain produces denser and more strongly clustered vegetation than grass terrain, while grass terrain produces sparse and isolated vegetation placements.

#### Scenario: Forest tiles produce denser placements than grass tiles
- **WHEN** deterministic vegetation sampling evaluates world-space candidates in forest and grass terrain regions for the same world seed
- **THEN** forest regions admit high-density vegetation placement with stronger clustering
- **AND** grass regions admit only low-density, occasional vegetation placement

### Requirement: Vegetation instances are anchored to world tiles and retain prototype geometry
The vegetation system SHALL publish vegetation instances as data-only records anchored at deterministic world tiles, with stable identity, prototype selection, world-space placement, and geometry derived from vegetation metadata rather than flattened ad hoc per-tile records.

#### Scenario: Multi-tile tree records keep anchor and footprint information
- **WHEN** the world publishes a large vegetation object such as a tree
- **THEN** the published instance identifies a stable anchor position and deterministic prototype selection
- **AND** it carries footprint or bounds information sufficient for visibility tests and rendering
- **AND** the published record remains data-only without introducing interaction or script state

### Requirement: Vegetation metadata resolves from the staged vegetation classification
The vegetation system SHALL load vegetation prototypes from the staged overworld vegetation tileset classification, using object identifiers, anchor roles, and part offsets to resolve multi-tile vegetation atlas metadata.

#### Scenario: Vegetation prototype metadata is assembled from anchor and part tiles
- **WHEN** runtime vegetation metadata is loaded from the staged overworld vegetation classification
- **THEN** the runtime groups tiles by object identifier
- **AND** it resolves anchor tiles and part offsets into prototype metadata that can be used for deterministic vegetation placement and rendering

### Requirement: Vegetation placement respects classification anchor constraints
The vegetation system SHALL evaluate vegetation prototype eligibility using placement metadata loaded from the staged vegetation classification. A prototype SHALL only be eligible for deterministic placement when the anchor tile's terrain class appears in the prototype's `placeOn` metadata and the prototype defines positive biome support for that terrain class.

#### Scenario: Water vegetation appears on valid water anchors
- **WHEN** deterministic vegetation sampling evaluates a water tile whose anchor class is allowed by a prototype such as a water lily or marsh reeds entry
- **THEN** the prototype is eligible for placement on that tile
- **AND** the runtime does not reject the placement only because the anchor tile is water

#### Scenario: Unsupported terrain classes remain ineligible
- **WHEN** deterministic vegetation sampling evaluates an anchor tile whose terrain class is not listed in a prototype's `placeOn` metadata or has no positive biome support
- **THEN** that prototype is not eligible for placement on that tile
- **AND** deterministic placement does not produce that prototype there

### Requirement: Vegetation placement remains deterministic while using metadata-driven prototype pools
The vegetation system SHALL preserve deterministic content identities, placement stability, and geometry while deriving prototype eligibility and prototype selection from loaded vegetation placement metadata instead of hardcoded terrain-specific prototype lists.

#### Scenario: Repeated metadata-driven vegetation queries stay stable
- **WHEN** the runtime resolves vegetation for the same world seed and the same world-space area multiple times
- **THEN** it returns the same metadata-qualified vegetation instances with the same identities, prototype selections, anchor positions, and geometry
- **AND** the result does not depend on prior chunk generation order or runtime random state

### Requirement: Visible vegetation queries include large objects whose bounds intersect the frame
The world-facing vegetation visibility query SHALL return vegetation instances whose rendered bounds intersect the active camera frame even when the owning anchor tile belongs to a nearby chunk outside the immediate visible chunk range.

#### Scenario: Tree canopy remains visible when its anchor sits just outside the immediate chunk range
- **WHEN** a large vegetation instance is anchored in a nearby retained chunk and its rendered bounds intersect the active camera frame
- **THEN** the world-visible vegetation query includes that instance
- **AND** the query does not require duplicating the instance into multiple owning chunks
