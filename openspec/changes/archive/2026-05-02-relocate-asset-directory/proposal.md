## Why

The project's runtime assets now live in the repository-root `assets/` directory instead of `src/main/assets/`, but the build, runtime asset lookups, tests, and documentation still assume the old layout. Updating that wiring now keeps local builds and tests aligned with the repository's actual asset location and avoids broken asset loading paths.

## What Changes

- Update executable asset staging to copy runtime assets from the repository-root `assets/` directory into the built executable's `assets/` directory.
- Update runtime-facing and test-facing path assumptions so asset-dependent code and checks resolve the relocated files consistently.
- Update repository documentation to describe the new asset layout and staged build output accurately.

## Capabilities

### New Capabilities
- `runtime-asset-layout`: Define how the project sources runtime assets from the repository root and stages them for the executable and asset-dependent tests.

### Modified Capabilities

## Impact

- Affected code: `src/main/CMakeLists.txt`, `tests/CMakeLists.txt`, and any runtime helpers or path utilities that assume the previous source-asset layout.
- Affected documentation: `README.md`.
- Affected systems: post-build asset staging, runtime asset lookup, and asset-dependent test inputs.
