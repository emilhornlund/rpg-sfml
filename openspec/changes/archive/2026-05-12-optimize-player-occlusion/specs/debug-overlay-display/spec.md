## MODIFIED Requirements

### Requirement: Debug overlay presents readable runtime diagnostics
The debug overlay SHALL render as a screen-anchored semi-transparent black panel with readable text showing frame rate, retained chunk count, retained generated-content count, rendered generated-content count, visible tile count, visible generated-content count, front occluder count, overlap-qualified occlusion candidate count, terrain vertex count, tile-grid vertex count, player tile coordinates, and zoom level.

#### Scenario: Drawing overlay diagnostics
- **WHEN** the debug overlay is visible during an overworld frame
- **THEN** the runtime draws a semi-transparent black background panel in screen space
- **AND** it renders text for frame rate, retained chunk count, retained generated-content count, rendered generated-content count, visible tile count, visible generated-content count, front occluder count, overlap-qualified occlusion candidate count, terrain vertex count, tile-grid vertex count, player tile coordinates, and zoom level on top of that panel

#### Scenario: Overlay reflects current cache state after chunk unloading
- **WHEN** the player explores far enough for the world cache to unload out-of-window chunks
- **THEN** the overlay diagnostics reflect the current retained chunk count and retained generated-content count after unloading
- **AND** the rendered generated-content count and visible generated-content count continue to describe only the objects visible in the active frame

#### Scenario: Overlay reports occlusion and render-batch scale
- **WHEN** the debug overlay is visible for an overworld frame with terrain, generated content, and player-occlusion diagnostics prepared for drawing
- **THEN** the overlay reports the visible tile count and terrain vertex count for the active frame
- **AND** it reports the broad front occluder count derived from the current render queue
- **AND** it reports the overlap-qualified occlusion candidate count derived from the active player-overlap test
- **AND** it reports the tile-grid vertex count for the current frame, using zero when the tile-grid overlay is disabled

#### Scenario: Overlay hides cleanly
- **WHEN** the debug overlay is toggled off
- **THEN** the runtime stops drawing the overlay panel and diagnostic text
- **AND** overworld scene rendering continues without leaving overlay artifacts on screen
