## 1. Vegetation metadata and asset support

- [x] 1.1 Add runtime asset support for the overworld vegetation tileset image and staged classification document.
- [x] 1.2 Implement vegetation metadata loading that groups classification entries into anchor-based object prototypes with bounds and atlas-part offsets.
- [x] 1.3 Add asset-loading and staging coverage for the vegetation tileset runtime files.

## 2. Deterministic vegetation sampling

- [x] 2.1 Extend world-content data structures to represent vegetation instances with stable identity, prototype selection, anchor placement, and bounds.
- [x] 2.2 Implement deterministic vegetation sampling from world-space seed/hash inputs with biome-sensitive density and clustering rules for forest and grass terrain.
- [x] 2.3 Retain vegetation by anchor-owning chunk and update visible-content queries to include nearby large objects whose bounds intersect the camera frame.

## 3. Snapshot publishing and render ordering

- [x] 3.1 Publish vegetation-ready generated-content entries and y-sort ordering data through the overworld render snapshot.
- [x] 3.2 Update overworld rendering support so terrain remains a terrain pass while vegetation content and player presentation render in deterministic y-sorted order.
- [x] 3.3 Replace placeholder generated-content drawing with vegetation atlas-based sprite rendering from snapshot-published metadata.

## 4. Validation coverage

- [x] 4.1 Add deterministic world-content tests covering repeated queries, biome-sensitive density differences, and large-object visibility across chunk boundaries.
- [x] 4.2 Update runtime snapshot and rendering support tests to verify vegetation publishing, staged asset availability, and y-sorted object rendering behavior.
