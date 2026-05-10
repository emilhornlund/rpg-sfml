## 1. Runtime viewport sizing

- [x] 1.1 Replace the fixed post-startup overworld viewport source in `Game` with the current window size so frame updates use live dimensions.
- [x] 1.2 Keep world rendering driven by the snapshot-owned camera frame while preserving the existing screen-space debug overlay behavior.

## 2. Snapshot and regression coverage

- [x] 2.1 Add or update focused tests that verify resized viewport dimensions propagate into overworld input or camera-frame sizing.
- [x] 2.2 Run the existing build and test commands to confirm the resize viewport change does not break current runtime behavior.
