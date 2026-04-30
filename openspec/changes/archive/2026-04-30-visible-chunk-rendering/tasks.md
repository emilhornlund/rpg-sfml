## 1. World render query boundary

- [x] 1.1 Add a world-facing visible-terrain query that derives tile and chunk traversal bounds from a `ViewFrame`.
- [x] 1.2 Implement bounded overscan and world-edge clipping for visible-terrain traversal, including partial edge chunks.
- [x] 1.3 Ensure visible render traversal reuses retained chunk data instead of introducing a separate generation path.

## 2. Runtime rendering integration

- [x] 2.1 Update `Game::render()` to consume the world-facing visible-terrain query instead of looping across the full world dimensions.
- [x] 2.2 Preserve existing tile coloring and player-marker rendering behavior while switching terrain drawing to visible traversal.

## 3. Verification

- [x] 3.1 Add or update tests for camera-bounded terrain traversal and clipping at overworld edges.
- [x] 3.2 Add or update tests proving repeated visible render queries reuse retained chunk data.
- [x] 3.3 Run the existing build and test commands after the rendering change is implemented.
