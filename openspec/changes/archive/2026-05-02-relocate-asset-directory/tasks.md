## 1. Build and test path updates

- [x] 1.1 Update `src/main/CMakeLists.txt` so the main executable stages runtime assets from the repository-root `assets/` directory.
- [x] 1.2 Update asset-dependent test configuration in `tests/CMakeLists.txt` to read checked-in runtime assets from the repository-root `assets/` directory while keeping staged-output checks pointed at the built executable's `assets/` directory.

## 2. Documentation and verification

- [x] 2.1 Update `README.md` to describe the repository-root `assets/` directory as the source asset location and `build/src/main/assets/` as the staged runtime output.
- [x] 2.2 Build the project and run the existing test suite to confirm the relocated asset paths are wired correctly.
