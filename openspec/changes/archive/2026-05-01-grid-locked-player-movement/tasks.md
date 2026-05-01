## 1. Input direction resolution

- [x] 1.1 Extend the overworld shell input path to preserve directional press ordering and expose one active cardinal movement direction using last-pressed-wins semantics.
- [x] 1.2 Update input translation tests to cover canceled opposite directions and perpendicular last-pressed-wins combinations without exposing SFML types to gameplay code.

## 2. Grid-locked player movement

- [x] 2.1 Refactor `Player` movement state to travel between adjacent tile centers instead of normalizing a free movement vector.
- [x] 2.2 Enforce auto-completion of an in-progress tile step, only start new steps from tile centers, and reject blocked destination tiles before movement begins.
- [x] 2.3 Preserve facing direction and walk animation behavior so presentation continues to reflect the active cardinal movement step.

## 3. Runtime integration and verification

- [x] 3.1 Keep overworld runtime, camera, and render snapshot integration working with the tile-step movement model.
- [x] 3.2 Update gameplay and runtime tests to verify tile-centered progression, mid-step input release completion, invalid-step rejection, and next-step turning behavior.
