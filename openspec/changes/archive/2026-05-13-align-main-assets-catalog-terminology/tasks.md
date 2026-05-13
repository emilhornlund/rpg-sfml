## 1. Update runtime asset contracts

- [x] 1.1 Rename emitted tileset artifact constants, helper paths, and staged asset references from classification-based names to catalog-based names in `src/main/CMakeLists.txt` and `src/main/GameAssetSupport.hpp`
- [x] 1.2 Update runtime loader-facing code and diagnostics to use catalog terminology for emitted documents where they refer to artifact files and paths, without renaming semantic tile `class` handling

## 2. Align asset-dependent tests

- [x] 2.1 Update CTest wiring and staging checks to expect catalog-based source and build-output asset paths
- [x] 2.2 Update asset-loading and helper tests to resolve the renamed catalog files while preserving existing assertions about wrapped JSON content and semantic tile metadata

## 3. Verify end-to-end asset consumption

- [x] 3.1 Reconfigure and build the project so the main executable stages the renamed catalog assets correctly
- [x] 3.2 Run the existing test suite to confirm runtime loading and asset-dependent checks pass with the catalog-based artifact layout
