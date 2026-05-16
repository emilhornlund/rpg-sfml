## 1. Extend overlay asset loading

- [x] 1.1 Teach `TilesetAssetLoader` to parse and preserve `overlay` tile entries from staged catalog documents.
- [x] 1.2 Add repo-native asset helpers and bootstrap wiring to load the ground overlay catalog metadata and runtime texture.
- [x] 1.3 Introduce a dedicated road overlay metadata helper that resolves base variants and surface-specific transition cells from the loaded catalog.

## 2. Publish road overlays through world and runtime state

- [x] 2.1 Add world-owned deterministic road overlay data and visible road queries without pushing texture details into gameplay modules.
- [x] 2.2 Extend `OverworldRenderSnapshot` with render-ready visible road overlay entries, including world geometry and underlying terrain context.
- [x] 2.3 Update runtime-facing tests or boundary checks for the expanded snapshot and asset-loading surface.

## 3. Render the road overlay layer

- [x] 3.1 Add batched road overlay vertex generation using road occupancy, neighboring road tiles, underlying terrain type, and deterministic base selection.
- [x] 3.2 Extend scene render resources and frame assembly to carry the road overlay texture, metadata, vertex array, and render states.
- [x] 3.3 Draw the road overlay pass after terrain and before vegetation/player rendering while preserving existing render-plan behavior.

## 4. Make deterministic content road-aware

- [x] 4.1 Prevent ground-dense vegetation placement from using road-covered tiles as anchors.
- [x] 4.2 Verify the first road-overlay slice stays deterministic and visually clean for repeated frames and repeated world loads.
