## 1. Camera framing contract

- [x] 1.1 Change the `Camera` public update contract so it depends only on focus position and viewport size, and remove world-size clamping from the camera implementation
- [x] 1.2 Update runtime call sites and camera-facing tests to use the world-independent camera API

## 2. Finite-world API and rendering cleanup

- [x] 2.1 Remove or de-emphasize finite-world accessors that are no longer needed for camera-facing flows while preserving finite slice generation and traversal behavior inside `World`
- [x] 2.2 Keep visible terrain enumeration finite, but update render behavior and tests so unclamped frames near map edges draw only generated terrain and leave the rest of the view as background

## 3. Verification

- [x] 3.1 Replace clamped-camera assertions with focus-following and edge-exposure assertions in overworld tests
- [x] 3.2 Run the existing build and test commands after the camera and world API changes are implemented
