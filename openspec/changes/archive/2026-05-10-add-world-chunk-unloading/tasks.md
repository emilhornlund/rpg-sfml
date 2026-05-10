## 1. World retention policy

- [x] 1.1 Add world-owned retention-window bookkeeping and a bounded chunk-pruning path that can evict retained chunks outside the active buffered window.
- [x] 1.2 Wire visible-frame refresh logic to establish the active retention window before terrain/content enumeration so the visible area and overscan remain retained.
- [x] 1.3 Preserve deterministic chunk regeneration after eviction and add focused tests covering unload/reload behavior.

## 2. Runtime diagnostics

- [x] 2.1 Extend the debug snapshot with retained chunk count and updated retained generated-content metrics that reflect the current cache state.
- [x] 2.2 Update the game debug overlay text so it presents retained chunks, retained objects, rendered objects, player coordinates, and zoom with clear labels.

## 3. Regression coverage

- [x] 3.1 Add tests proving retained counts can stabilize or decrease after moving the active window away from earlier chunks.
- [x] 3.2 Add tests proving visible terrain/content queries still return the required data while chunk unloading is enabled.
- [x] 3.3 Run the existing build and CTest workflow after the implementation changes.
