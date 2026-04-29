## Context

The current runtime opens an SFML window, owns the main loop, and already instantiates dedicated `World`, `Player`, and `Camera` module types. Those modules are still empty, so the executable has no playable scene and no concrete gameplay-facing data flow between simulation and rendering.

This change adds a narrow vertical slice that proves the existing runtime shape can support a deterministic overworld without collapsing gameplay logic back into `Game`. The design must keep `Game` as the coordinator, preserve the public `Game` boundary, avoid new external dependencies, and stay simple enough to test with the existing lightweight CTest approach.

## Goals / Non-Goals

**Goals:**
- Generate the same overworld tile grid from the same built-in seed on every run.
- Move world-specific state and tile classification into `World`.
- Move player position and movement rules into `Player`.
- Move follow/framing behavior into `Camera`.
- Render a basic tile map and player marker through the existing runtime loop.
- Keep enough logic outside direct SFML window code that deterministic behavior can be tested.

**Non-Goals:**
- Chunk streaming, infinite worlds, save data, or configurable seeds.
- Collision against rich world objects, combat, UI, inventory, or NPC systems.
- External asset loading, sprite sheets, animation systems, or content pipelines.
- Sophisticated camera smoothing, zoom systems, or multiple view modes.

## Decisions

### Keep one fixed deterministic generation path for the slice

The slice will use a single built-in seed and a lightweight procedural rule set so every launch produces the same tile layout. This keeps the first vertical slice reproducible for tests and review while still exercising world generation as a real system.

- **Why this approach:** deterministic output gives the project stable expectations for tests and avoids introducing configuration, persistence, or UI before the slice is proven out.
- **Alternative considered:** runtime-configurable seeds. Rejected because it broadens scope and weakens repeatability for the first slice.

### Let `World` own tile data and generation outputs

`World` will own the map dimensions, tile storage, spawn-safe tile selection, and any helper queries needed by movement and rendering. `Game` will ask `World` for the data it needs instead of deriving map state itself.

- **Why this approach:** it matches the existing gameplay-module boundary and prevents world logic from leaking into the loop coordinator.
- **Alternative considered:** storing tile data directly in `Game::Impl`. Rejected because it would undermine the existing `World` boundary and make later world expansion harder.

### Let `Player` own intent-to-position updates against world constraints

`Player` will track the player's tile or world position and expose a small update surface driven by input intent and elapsed time. Movement validation will consult `World` so the player can stay within the generated map and any impassable tile rules defined for the slice.

- **Why this approach:** it keeps input interpretation and movement behavior local to the player module while leaving terrain rules in the world module.
- **Alternative considered:** making `Game` mutate player coordinates directly during event handling. Rejected because it mixes orchestration with gameplay rules.

### Let `Camera` produce the current framing from player state

`Camera` will derive the visible framing from the player's current position and clamp the view to the world bounds. `Game` will apply that framing during rendering through SFML view state without exposing SFML types in the public `Game` header.

- **Why this approach:** camera behavior remains isolated and can evolve independently from player and world logic.
- **Alternative considered:** rendering everything in screen space with no camera abstraction. Rejected because the requested slice explicitly includes camera behavior and the project already has a dedicated module for it.

### Use basic built-in tile rendering instead of an asset pipeline

The first slice will render a small set of terrain categories with simple shapes or colored quads. This keeps the scope on deterministic world behavior and module interaction rather than asset ingestion.

- **Why this approach:** it satisfies visible tile rendering with minimal dependency and tooling overhead.
- **Alternative considered:** adding sprite assets and texture atlases now. Rejected because it introduces unnecessary content-pipeline work for the first slice.

### Keep deterministic simulation helpers testable outside the full window loop

Procedural generation rules, movement constraints, and camera clamping should live in module code or small runtime helpers that can be exercised in tests without depending on interactive window behavior.

- **Why this approach:** the repository already favors lightweight logic tests, and deterministic slice behavior is easier to maintain when its rules are directly testable.
- **Alternative considered:** validating everything only through manual runtime behavior. Rejected because it would make regressions harder to catch.

## Risks / Trade-offs

- **[Risk]** A fixed seed can make the slice feel less “procedural” than a configurable generator. **→ Mitigation:** define the generator so the seed can become a future extension point without changing the slice contract.
- **[Risk]** Tile-by-tile rendering may not scale if map size grows later. **→ Mitigation:** keep the first slice map intentionally modest and defer render batching optimizations until world size expands.
- **[Risk]** Mixing tile-space and world-space coordinates across modules can create subtle follow or movement bugs. **→ Mitigation:** document and implement one canonical coordinate model for the slice and convert only at render boundaries.
- **[Risk]** Camera clamping against small maps can produce awkward framing edge cases. **→ Mitigation:** define world dimensions and viewport behavior together so the camera always produces valid bounds.

## Migration Plan

No data or deployment migration is required. The change is an in-place runtime expansion of the current executable and test suite.

## Open Questions

None for the proposal scope. The slice can proceed with a fixed seed, simple terrain categories, and immediate camera follow behavior.
