## 1. Overworld runtime boundary

- [x] 1.1 Add the dedicated overworld runtime/session type and introduce repo-native input and render-facing state types for the current overworld slice.
- [x] 1.2 Define the overworld runtime public surface so it can initialize the active session, advance one frame from input, and expose the current render-facing frame state.
- [x] 1.3 Keep `World`, `Player`, and `Camera` as the owned gameplay modules behind that new boundary instead of moving their internal rules into the coordinator.

## 2. Move overworld orchestration out of Game

- [x] 2.1 Move player spawn initialization, player update sequencing, and camera follow sequencing from `Game.cpp` into the dedicated overworld runtime.
- [x] 2.2 Move visible-terrain and player-marker frame assembly behind the overworld runtime so `Game.cpp` renders from gameplay-provided frame state.
- [x] 2.3 Keep SFML event polling, window lifecycle handling, and backend drawing in `Game` while translating current frame input into the overworld runtime boundary.

## 3. Verification and boundary checks

- [x] 3.1 Update or add focused tests that prove the overworld runtime owns session initialization, gameplay progression, and render-facing frame data for the existing overworld slice.
- [x] 3.2 Update boundary-oriented checks so `Game` remains a thin shell and does not re-absorb direct overworld orchestration details.
- [x] 3.3 Run the existing build and test commands after the runtime extraction is implemented.
