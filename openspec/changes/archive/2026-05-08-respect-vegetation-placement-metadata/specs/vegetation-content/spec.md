## ADDED Requirements

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
