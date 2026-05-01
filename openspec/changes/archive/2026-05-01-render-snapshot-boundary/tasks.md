## 1. Snapshot contract

- [x] 1.1 Introduce dedicated overworld render snapshot types for camera framing, visible tile entries, and renderable marker entries in the shared runtime-facing headers.
- [x] 1.2 Replace the player-only render state shape with a marker collection that can represent the player now and additional entity markers later without SFML dependencies.

## 2. Gameplay snapshot assembly

- [x] 2.1 Update `OverworldRuntime` to build and expose the current render snapshot during initialization and per-frame updates.
- [x] 2.2 Keep visible tile traversal and marker placement behind the gameplay/runtime boundary so snapshot consumers do not query `World`, `Player`, or `Camera` directly.

## 3. Game rendering handoff

- [x] 3.1 Refactor `Game` to read the published render snapshot, configure the view from the snapshot camera frame, and draw tiles and markers from snapshot entries.
- [x] 3.2 Remove ad hoc overworld presentation derivation from `Game` so it remains an SFML shell around runtime input, window lifecycle, and backend drawing.

## 4. Verification

- [x] 4.1 Add or update focused tests covering render snapshot assembly and the runtime shell handoff.
- [x] 4.2 Run the existing configure, build, and test commands to confirm the snapshot boundary integrates cleanly.
