## 1. Asset and metadata setup

- [x] 1.1 Update runtime, build, and test asset wiring to stage `overworld-terrain-tileset.png` and `overworld-terrain-tileset-classification.json`.
- [x] 1.2 Add a render-side terrain atlas metadata loader that parses the classification JSON and validates required base, transition, and animated water entries during startup.

## 2. Autotile selection logic

- [x] 2.1 Implement canonical directed terrain-pair normalization and deterministic mixed-neighbor priority for the supported terrain transitions.
- [x] 2.2 Implement the fourteen-role autotile resolver from neighboring terrain ownership so a visible tile can resolve `single_tile`, edge, center, and outer-corner roles deterministically.
- [x] 2.3 Implement deterministic base-variant lookup for grass, sand, and forest plus a canonical open-water interior lookup strategy for water-targeting transitions.

## 3. Rendering integration

- [x] 3.1 Replace the fixed `TileType -> atlas cell` lookup in the game shell with metadata-driven base, transition, and role selection while preserving the existing render snapshot boundary.
- [x] 3.2 Add animated water transition frame selection using a shared animation phase without changing world generation or gameplay-facing terrain types.

## 4. Tests and verification

- [x] 4.1 Add focused tests for terrain atlas metadata loading and required-entry validation.
- [x] 4.2 Add focused tests for pair normalization, autotile role selection, deterministic base variation, canonical open-water lookup, and water animation frame selection.
