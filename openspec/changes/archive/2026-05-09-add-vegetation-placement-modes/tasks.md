## 1. Placement metadata loading

- [x] 1.1 Extend vegetation metadata loading so anchor `placementMode` values are preserved on `VegetationPrototype` records.
- [x] 1.2 Add or update asset-loading tests to verify representative anchors retain `placementMode` alongside existing `placeOn` and biome metadata.

## 2. Placement-mode-driven spawning

- [x] 2.1 Refactor world content generation to choose placement behavior from `placementMode` instead of `family`, tags, or prototype names.
- [x] 2.2 Keep biome weights as prototype-selection weights within each placement mode's eligible pool.
- [x] 2.3 Add a sparse `prop_sparse` placement path so forests produce occasional props while preserving many trees and some dense ground cover.

## 3. Verification

- [x] 3.1 Add or update world-generation tests to verify forest stumps/logs still appear but remain rarer than dense ground cover.
- [x] 3.2 Run the existing build and test commands to confirm the placement-mode change integrates cleanly.
