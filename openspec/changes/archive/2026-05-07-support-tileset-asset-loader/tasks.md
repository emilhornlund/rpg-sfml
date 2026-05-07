## 1. Asset layout and staging

- [x] 1.1 Update runtime asset path helpers and build staging so terrain classifications are sourced from `assets/output/classifications/` and staged correctly with the executable.
- [x] 1.2 Update asset-dependent tests and staging checks to reference the generated classification source layout and expected staged runtime paths.

## 2. Shared tileset asset loading

- [x] 2.1 Add a runtime-side tileset asset document/loader that parses wrapped classification documents and exposes tileset metadata plus tile entries.
- [x] 2.2 Implement asset-root-based path resolution for classification files and referenced tileset images so staged output directories and root images work together.

## 3. Terrain metadata migration

- [x] 3.1 Refactor terrain metadata construction to consume the shared tileset asset document instead of the legacy flat regex parser.
- [x] 3.2 Preserve terrain base-variant, decor, transition-role, and water-animation behavior while sourcing atlas cells from nested wrapped-schema terrain entries.

## 4. Verification and future-ready coverage

- [x] 4.1 Extend terrain metadata tests to exercise the wrapped generated terrain classification through the new loader path.
- [x] 4.2 Add focused coverage or validation for loading additional known classifications, including vegetation, without enabling vegetation rendering yet.
