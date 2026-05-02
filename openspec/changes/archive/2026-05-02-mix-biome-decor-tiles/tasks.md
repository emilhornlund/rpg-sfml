## 1. Metadata support

- [x] 1.1 Extend terrain tileset metadata loading to retain biome decor atlas cells separately from required base variants.
- [x] 1.2 Add metadata accessors and validation rules so base variants stay required while decor remains optional per biome.

## 2. Deterministic terrain appearance selection

- [x] 2.1 Replace the non-transition base-only fallback with a deterministic base-or-decor selector derived from world seed, tile coordinates, and terrain category.
- [x] 2.2 Keep autotile transition resolution authoritative so transition tiles always override the default base-or-decor appearance path.
- [x] 2.3 Apply a conservative decor weight so decor appears as a low-frequency biome-specific replacement instead of dominating the terrain.

## 3. Verification

- [x] 3.1 Update terrain autotile metadata and atlas-selection tests to cover decor loading, deterministic selection stability, and transition precedence.
- [x] 3.2 Run the existing build and test commands to confirm the new terrain appearance selection integrates cleanly with the current overworld runtime.
