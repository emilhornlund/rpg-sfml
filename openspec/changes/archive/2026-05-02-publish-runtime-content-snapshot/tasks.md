## 1. World-visible content queries

- [x] 1.1 Add a world-facing visible generated-content query that enumerates retained chunk content intersecting an active `ViewFrame`.
- [x] 1.2 Reuse or extract frame/chunk visibility helpers so visible terrain and visible generated content stay aligned and footprint-aware.

## 2. Render snapshot publication

- [x] 2.1 Extend the overworld render snapshot types with a dedicated generated-content render entry shape and collection.
- [x] 2.2 Update `OverworldRuntime::refreshRenderSnapshot()` to merge visible terrain, player presentation, and visible generated content into the published snapshot.

## 3. Shell rendering and verification

- [x] 3.1 Update the SFML shell rendering flow to draw generated content from snapshot-published entries between terrain and player rendering.
- [x] 3.2 Extend runtime and world tests to cover visible generated-content queries, snapshot publication, and render-facing content metadata.
- [x] 3.3 Run the documented build and test commands to confirm the new snapshot path integrates cleanly.
