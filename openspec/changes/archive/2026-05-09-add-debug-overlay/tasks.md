## 1. Runtime asset and input setup

- [x] 1.1 Update runtime asset staging so the main executable copies one bundled font from `assets/output/fonts/` into the built executable's `assets/` directory.
- [x] 1.2 Extend shell-side input handling in `Game` so `F1` toggles debug overlay visibility without interfering with existing movement, zoom, grid, or exit controls.

## 2. Gameplay-facing debug snapshot

- [x] 2.1 Add repo-native debug snapshot types and accessors to `OverworldRuntime` for player tile coordinates, zoom percentage, loaded generated-content count, and rendered generated-content count.
- [x] 2.2 Populate the debug snapshot during initialization and per-frame updates using gameplay-owned `World`, `Player`, and `Camera` state while preserving the existing render snapshot boundary.

## 3. Shell-side overlay rendering

- [x] 3.1 Load the staged font asset in `Game` and track a readable frame-rate value at the shell boundary.
- [x] 3.2 Render a screen-anchored semi-transparent black overlay panel and diagnostic text after the overworld scene when the overlay visibility flag is enabled.

## 4. Verification

- [x] 4.1 Add or update focused tests for the new asset-staging expectation and any repo-native debug snapshot or input translation helpers introduced by the change.
- [x] 4.2 Build the project and run the existing test suite to confirm the overlay change integrates cleanly with the current runtime.
