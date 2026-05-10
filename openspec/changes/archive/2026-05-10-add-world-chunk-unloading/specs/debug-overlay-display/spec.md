## MODIFIED Requirements

### Requirement: Debug overlay presents readable runtime diagnostics
The debug overlay SHALL render as a screen-anchored semi-transparent black panel with readable text showing frame rate, retained chunk count, retained generated-content count, rendered generated-content count, player tile coordinates, and zoom level.

#### Scenario: Drawing overlay diagnostics
- **WHEN** the debug overlay is visible during an overworld frame
- **THEN** the runtime draws a semi-transparent black background panel in screen space
- **AND** it renders text for frame rate, retained chunk count, retained generated-content count, rendered generated-content count, player tile coordinates, and zoom level on top of that panel

#### Scenario: Overlay reflects current cache state after chunk unloading
- **WHEN** the player explores far enough for the world cache to unload out-of-window chunks
- **THEN** the overlay diagnostics reflect the current retained chunk count and retained generated-content count after unloading
- **AND** the rendered generated-content count continues to describe only the objects visible in the active frame
