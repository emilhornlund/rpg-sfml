## Context

The repository-root assets now ship generated tileset documents under `assets/output/catalogs/` with `*-catalog.json` filenames, but the main executable and its tests still refer to the previous classification-based artifact paths. This mismatch affects three connected surfaces:

- build staging in `src/main/CMakeLists.txt`
- runtime asset lookup helpers and loader-facing paths in `src/main/`
- asset-dependent tests and CTest wiring in `tests/`

The assets submodule already documented the intended compatibility boundary: `catalog` is the public name for emitted JSON artifacts, while semantic tile classification inside the JSON payload remains unchanged. The main repository needs to adopt that same split so it can consume the updated assets without rewriting schema semantics.

## Goals / Non-Goals

**Goals:**

- Align the main repository with the assets submodule's catalog-based output paths and filenames.
- Update runtime staging, lookup helpers, and asset-dependent tests to use one canonical emitted artifact layout.
- Preserve the wrapped JSON schema and semantic terrain/object metadata already consumed by runtime code.

**Non-Goals:**

- Renaming semantic concepts such as terrain `class` values or internal runtime branching that still models classification.
- Changing the generated JSON document structure, field names, or schema version.
- Introducing compatibility aliases for both old and new asset paths.

## Decisions

### Treat artifact renames as a path contract update

The implementation will rename directory and filename references that identify emitted tileset documents, moving from `output/classifications/` and `*-classification.json` to `output/catalogs/` and `*-catalog.json`.

This keeps the change focused on the actual breaking surface exposed by the assets submodule.

Alternative considered: support both path families. Rejected because it would preserve two public contracts and weaken the goal of a single canonical layout.

### Preserve semantic classification data in loader-facing models

The runtime will continue parsing terrain `class` values and any existing internal fields or branching that represent classification semantics. The proposal only changes how emitted files are named and located, not how tile metadata is interpreted after loading.

Alternative considered: rename internal data members and parser terminology everywhere. Rejected because the current asset JSON still uses semantic `class` data, and a broad internal rename would mix a path migration with an unrelated schema/model refactor.

### Update build, runtime, and tests together

The change will cover CMake staging, runtime asset path helpers, CTest arguments, and source-backed/staged asset assertions in one pass so the repository presents a consistent catalog-based contract.

Alternative considered: update only runtime code and leave tests/build wiring for follow-up. Rejected because the build and tests are part of the same asset contract and would remain broken.

## Risks / Trade-offs

- **A remaining classification-path reference keeps the build or tests broken** → Update every path consumer in CMake, runtime helpers, and asset-dependent tests together.
- **Over-renaming semantic code obscures the difference between emitted artifacts and tile metadata** → Limit required renames to artifact-oriented constants, helper names, messages, and paths; leave semantic `class` parsing intact.
- **Error text and helper names become temporarily mixed** → Prefer catalog terminology when referring to emitted documents, but avoid risky churn in code paths whose semantics are still classification-oriented.
