## Context

The repository now has a root-level `assets/` directory that contains the runtime images and metadata used by the executable. The current build and test wiring still points at `src/main/assets/`, while the runtime itself already loads assets from an executable-local `assets/` directory after staging.

This is a cross-cutting but low-risk change because it touches executable CMake wiring, asset-dependent tests, and documentation together. Capturing the intended boundary avoids accidentally changing runtime behavior that is already correct.

## Goals / Non-Goals

**Goals:**
- Source bundled runtime assets from the repository-root `assets/` directory.
- Preserve the executable-local staged asset layout used by the runtime at run time.
- Update asset-dependent tests and documentation so they describe the new source layout accurately.

**Non-Goals:**
- Changing runtime asset filenames or asset contents.
- Introducing an asset manifest, asset discovery system, or data-driven asset registration.
- Changing the executable's staged asset directory away from `assets/` next to the built binary.

## Decisions

### Repository assets become the single source for build staging

`src/main/CMakeLists.txt` will stage the known runtime assets from `${PROJECT_SOURCE_DIR}/assets` instead of from a source-directory-local asset folder. This keeps the build aligned with the repository's new asset location while preserving the existing explicit list of staged files.

**Alternatives considered:**
- **Keep sourcing assets from `src/main/assets` and duplicate files at the root**: rejected because it preserves two competing source locations and leaves future edits ambiguous.
- **Introduce directory-wide globbing for assets**: rejected because the current build uses an explicit staged-file list, which is more reviewable and stable for this small project.

### Runtime asset lookup stays executable-relative

`Game.cpp` already resolves textures and metadata from an `assets/` directory next to the executable. That boundary will remain unchanged so the relocation affects source layout and staging only, not the game's runtime lookup contract.

**Alternatives considered:**
- **Load directly from `${PROJECT_SOURCE_DIR}/assets` at run time**: rejected because it would couple the executable to the source tree and break the existing staged-runtime model.

### Tests distinguish repository-source assets from staged runtime assets

Tests that need to read checked-in asset files directly will point at `${PROJECT_SOURCE_DIR}/assets/...`, while staging checks will continue to assert the built copies under `${PROJECT_BINARY_DIR}/src/main/assets/...`. This keeps the source and runtime concerns explicit.

**Alternatives considered:**
- **Move all tests to use only staged build assets**: rejected because some tests are validating checked-in metadata inputs rather than post-build copying behavior.

### Documentation should describe both source and staged locations

The README will describe the repository-root asset directory as the authoring/source location and `build/src/main/assets/` as the staged runtime location produced by the build.

## Risks / Trade-offs

- **[Risk] The old `src/main/assets/` directory may still exist temporarily and mask mistakes** → Mitigation: update every build and test reference to the root asset directory so the code no longer depends on the legacy path.
- **[Risk] Future contributors may assume the runtime reads directly from the repository root** → Mitigation: document the distinction between repository-source assets and executable-local staged assets.
- **[Risk] Adding new runtime assets could miss the explicit staged-file list** → Mitigation: keep the staging list centralized in `src/main/CMakeLists.txt` and update tests that protect staged asset availability.

## Migration Plan

1. Update executable asset staging to source files from the repository-root `assets/` directory.
2. Update asset-dependent tests to use the relocated source asset paths while preserving staged-output assertions.
3. Update README references to reflect the new source asset location and the unchanged staged runtime location.

Rollback is straightforward: restore the previous source asset paths in CMake, tests, and documentation.

## Open Questions

- None.
