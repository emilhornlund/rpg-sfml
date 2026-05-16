## ADDED Requirements

### Requirement: Ground vegetation placement respects road-covered tiles
Deterministic ground vegetation placement SHALL treat road-covered tiles as ineligible anchor tiles for ground-dense vegetation so visible roads remain readable.

#### Scenario: Ground cover is suppressed on roads
- **WHEN** deterministic ground vegetation sampling evaluates a tile covered by a road overlay
- **THEN** the runtime does not place ground-dense vegetation on that tile
- **AND** the suppression remains deterministic for the same world seed and world-space coordinates
