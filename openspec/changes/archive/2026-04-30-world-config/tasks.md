## 1. World configuration API

- [x] 1.1 Add a `WorldConfig` value type that captures seed, width in tiles, height in tiles, and tile size at the `World` module boundary
- [x] 1.2 Update `World` construction so callers can provide explicit configuration while the default construction path still preserves today's built-in world settings

## 2. Config-driven world behavior

- [x] 2.1 Refactor world generation to derive tile classification, spawn selection, world size, and coordinate conversion from the active `WorldConfig` instead of hardcoded constants
- [x] 2.2 Update runtime integration so the default overworld startup path continues to work with the default configuration and rendering/camera logic stays aligned with configured dimensions and tile size

## 3. Verification

- [x] 3.1 Extend overworld tests to cover deterministic generation for identical configurations and changed dimensions or tile size for alternate configurations
- [x] 3.2 Update or add tests for the default-configuration path so the runtime still gets a deterministic playable overworld without explicit inputs
