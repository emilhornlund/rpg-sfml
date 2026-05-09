## MODIFIED Requirements

### Requirement: Vegetation placement follows biome-specific density rules
The vegetation system SHALL apply biome-sensitive placement rules using explicit placement modes so forest terrain produces many `tree_sparse` placements, some `ground_dense` placements, and only occasional `prop_sparse` placements, while grass terrain remains sparser overall.

#### Scenario: Forest tiles produce a placement-mode density hierarchy
- **WHEN** deterministic vegetation sampling evaluates forest terrain candidates for the same world seed
- **THEN** `tree_sparse` vegetation is admitted more frequently than `ground_dense` vegetation
- **AND** `ground_dense` vegetation is admitted more frequently than `prop_sparse` vegetation

#### Scenario: Grass terrain remains sparser than forest terrain
- **WHEN** deterministic vegetation sampling evaluates grass and forest terrain regions for the same world seed
- **THEN** grass regions admit fewer placements overall than forest regions
- **AND** the density difference is achieved through placement-mode-driven rules rather than a single shared non-tree pass

### Requirement: Vegetation placement remains deterministic while using metadata-driven prototype pools
The vegetation system SHALL preserve deterministic content identities, placement stability, and geometry while deriving prototype eligibility, placement behavior, and prototype selection from loaded vegetation placement metadata instead of hardcoded terrain-specific prototype lists or prototype-name heuristics.

#### Scenario: Repeated metadata-driven vegetation queries stay stable
- **WHEN** the runtime resolves vegetation for the same world seed and the same world-space area multiple times
- **THEN** it returns the same metadata-qualified vegetation instances with the same identities, prototype selections, anchor positions, and geometry
- **AND** the result does not depend on prior chunk generation order or runtime random state

#### Scenario: Placement mode controls spawn behavior without name heuristics
- **WHEN** deterministic vegetation sampling evaluates a prototype with explicit `placementMode` metadata
- **THEN** the runtime chooses the placement pass from `placementMode`
- **AND** it does not infer sparse-vs-dense spawn behavior from prototype ids, tags, or the `family` field alone

## ADDED Requirements

### Requirement: Biome weights select prototypes within a placement mode
The vegetation system SHALL treat biome weights as prototype-selection weights within a placement mode's eligible pool after a placement pass has admitted a candidate location.

#### Scenario: Sparse forest props remain occasional while still selectable
- **WHEN** a forest `prop_sparse` candidate location passes its sparse placement rule
- **THEN** the runtime selects among eligible forest `prop_sparse` prototypes using their forest biome weights
- **AND** those weights do not by themselves make `prop_sparse` candidates as common as dense ground cover
