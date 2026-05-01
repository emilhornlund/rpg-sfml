## 1. Introduce the generation-policy boundary

- [x] 1.1 Add internal world-generation types/helpers for deterministic biome sampling and chunk terrain generation without changing the public `World` API
- [x] 1.2 Wire `World` to request missing chunk content through the new generation collaborator instead of owning the generation policy directly

## 2. Preserve world-owned runtime state and cache behavior

- [x] 2.1 Keep retained chunk storage, cache reuse, and world-facing tile/render queries owned by `World`
- [x] 2.2 Ensure spawn selection and any chunk-generated runtime payload remain derived through the generation-policy flow while staying exposed through existing world-facing queries

## 3. Lock in compatibility

- [x] 3.1 Update or add focused tests covering deterministic chunk lookup, retained chunk reuse, and unchanged runtime-facing world behavior after the split
- [x] 3.2 Update build wiring for any new internal generation compilation units needed by the refactor
