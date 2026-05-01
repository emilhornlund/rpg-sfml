## Context

`OverworldRuntime` already owns the gameplay-facing collaboration between `World`, `Player`, and `Camera`, and it already exposes an `OverworldFrameState` that `Game` reads during rendering. The remaining leak is that `Game` still interprets gameplay presentation details directly in its render path by deciding how to map tile state and marker state into draw calls instead of consuming a clearly named gameplay-owned snapshot boundary.

This change crosses the runtime shell, overworld runtime, and shared overworld types. It needs a design document because the goal is not just a field rename: it is to make snapshot assembly an explicit gameplay/runtime responsibility and to keep `Game` limited to SFML-facing rendering concerns.

## Goals / Non-Goals

**Goals:**
- Define a render snapshot contract owned by gameplay-facing runtime code.
- Ensure the snapshot contains the camera frame, visible tiles, and renderable marker data needed for the current overworld frame.
- Keep SFML types out of the gameplay snapshot contract.
- Make `Game` consume snapshots as a renderer instead of deriving presentation state ad hoc from gameplay internals.

**Non-Goals:**
- Introducing a general-purpose rendering engine or scene graph.
- Replacing SFML rendering primitives or changing window/event ownership.
- Expanding the scope beyond the active overworld frame contract.

## Decisions

### Introduce a dedicated overworld render snapshot type

`OverworldFrameState` is close to the intended shape already, but the new boundary should be framed as a snapshot contract rather than incidental frame state. The runtime-facing API will expose a clearly named snapshot structure containing:
- camera frame data
- visible terrain tile entries
- player and other entity marker entries
- sizing or presentation identifiers required to draw those entries without SFML dependencies

This keeps the gameplay side responsible for describing *what* should be drawn each frame while leaving `Game` responsible for *how* SFML draws it.

**Alternatives considered:**
- Keep `OverworldFrameState` as-is and only rename fields in `Game`: rejected because it would preserve the ad hoc boundary and not make the rendering contract explicit.
- Push SFML colors or shapes into gameplay data: rejected because it would violate the existing SFML boundary.

### Assemble the snapshot inside `OverworldRuntime`

`OverworldRuntime` already has the world/player/camera state required to build a complete frame snapshot. Snapshot assembly will stay there, potentially with small helpers for tile and marker conversion if that improves readability. `Game` will request the latest snapshot and iterate over render entries without querying gameplay modules directly.

**Alternatives considered:**
- Have `Game` pull world/player/camera state independently: rejected because it keeps presentation derivation in the renderer.
- Move snapshot assembly into `World`: rejected because the snapshot spans camera, player, and world concerns rather than a world-only responsibility.

### Represent markers as a renderable collection

The snapshot should describe the player marker using the same general mechanism that future entity markers can use. Even if the first implementation still emits only the player marker, the boundary will use a marker collection rather than a player-only special case so the runtime contract can grow without another shell refactor.

**Alternatives considered:**
- Keep a dedicated `playerMarker` field forever: rejected because the user request explicitly points toward player/entity markers and this would preserve a special-case API.

## Risks / Trade-offs

- **[Risk]** Snapshot types may duplicate some existing overworld data shapes. → **Mitigation:** Reuse existing value types such as `ViewFrame`, `WorldPosition`, and `WorldSize` where they already match the boundary.
- **[Risk]** Marker presentation data could become too renderer-specific. → **Mitigation:** Keep the contract limited to repo-native geometry and presentation identifiers, not SFML colors or drawable types.
- **[Risk]** Renaming/restructuring frame state could create noisy edits. → **Mitigation:** Keep the change narrow to `Game`, `OverworldRuntime`, shared types, and directly affected tests.

## Migration Plan

1. Introduce the snapshot types and adjust `OverworldRuntime` to publish them.
2. Update `Game` to render exclusively from the snapshot contract.
3. Update or add focused tests that verify snapshot assembly and the runtime shell handoff.
4. Remove or rename superseded frame-state APIs once snapshot consumption is in place.

Rollback is straightforward because the change is internal to the executable/runtime boundary and can be reverted by restoring the prior frame-state API if needed.

## Open Questions

None.
