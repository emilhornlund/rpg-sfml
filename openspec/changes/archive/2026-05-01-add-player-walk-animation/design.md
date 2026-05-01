## Context

The current overworld presents terrain through a tileset-backed render path, but the player is still drawn as a generic marker. The repository already contains a player walking spritesheet at `src/main/assets/player-walking-spritesheet.png`, and prior exploration established that the sheet is arranged as a 3x4 grid of 48x48 padded cells with rows ordered down, left, right, up. The visible character art is smaller than the full cell and sits above transparent padding, so placement must be based on a stable pivot rather than on the raw frame bounds.

This change crosses the `Player`, `OverworldRuntime`, render snapshot types, `Game`, asset staging, and tests. It therefore benefits from design capture before implementation.

## Goals / Non-Goals

**Goals:**
- Replace the player marker with sprite-backed rendering in the overworld.
- Keep gameplay modules free of SFML texture and sprite ownership.
- Let gameplay own directional facing and walk animation state.
- Define stable tile alignment for padded 48x48 frames on a 16x16 world tile baseline.
- Preserve the existing render snapshot boundary so `Game` renders from snapshot data rather than inspecting gameplay state directly.

**Non-Goals:**
- Adding animation support for non-player entities.
- Introducing a general-purpose asset manifest or animation authoring system.
- Changing player movement speed, collision, or camera rules.
- Supporting multiple player spritesheets or runtime skin swapping in this change.

## Decisions

### Gameplay owns facing and animation state

`Player` will own presentation-adjacent gameplay state that is still renderer-agnostic: facing direction, whether the player is currently moving, and the selected walk-frame index. This keeps animation timing tied to gameplay movement while avoiding SFML types in gameplay code.

**Alternatives considered:**
- **Drive animation entirely in `Game`**: rejected because it would require `Game` to infer player-facing and movement presentation from raw positions every frame, weakening the gameplay/module boundary.
- **Store SFML sprite state inside `Player`**: rejected because it breaks the established texture-agnostic gameplay boundary.

### Overworld snapshots publish sprite draw metadata, not texture objects

`OverworldRuntime` will continue to publish repo-native render data. The player marker entry will evolve from a generic marker concept into player sprite draw metadata that includes world-space placement plus animation selectors needed to choose the correct spritesheet cell.

The snapshot will identify:
- sprite frame size in world-space units
- origin/pivot in world-space units
- draw position in world-space units
- facing direction
- animation frame selection

`Game` remains responsible for loading the spritesheet texture and converting snapshot metadata into SFML sprite configuration.

**Alternatives considered:**
- **Expose atlas rectangles directly from gameplay**: rejected because it leaks asset-layout decisions into gameplay-facing types.
- **Keep generic marker appearance enums and infer frame choice in `Game`**: rejected because it duplicates gameplay presentation rules in the shell.

### Player alignment uses a foot-pivot anchored to the logical player position

The existing `playerPosition` already represents the player's world-space point on the tile grid. For sprite rendering, that point will be treated as the player's foot pivot. The padded 48x48 frame will therefore be placed so its origin lands on the player position while the visible character extends upward into the tile above.

This design avoids centering the full 48x48 cell on the tile, which would visibly sink the sprite into the ground because the lower portion of the cell is transparent padding.

**Alternatives considered:**
- **Center the full frame on the tile center**: rejected because the padded cell would place too much empty area below the feet.
- **Anchor at frame bottom-center**: rejected because the art does not extend to the bottom of the padded 48x48 cell.

### The sheet is treated as a fixed 3x4 directional walk sheet for this change

Implementation will use a fixed interpretation of the supplied asset:
- 3 columns per row
- 4 rows
- row order: down, left, right, up
- one idle frame derived from the middle column
- walking frames selected from the three-frame row while moving

This keeps the first implementation simple and sufficient for the known asset.

**Alternatives considered:**
- **Add data-driven sheet descriptors**: rejected as over-scoped for a single bundled asset.
- **Assume 16x32 raw cells**: rejected because the actual PNG dimensions show padded 48x48 cells.

### Asset staging remains executable-local

The player spritesheet will be staged through the same `src/main/CMakeLists.txt` post-build asset copy flow used for the terrain tileset. This keeps runtime asset lookup consistent with existing executable-relative loading.

## Risks / Trade-offs

- **[Risk] Padded-frame pivot values may need small visual tuning** → Mitigation: keep pivot constants centralized and add tests around world-space placement semantics so visual adjustments remain isolated.
- **[Risk] Animation timing may look too fast or too slow at the current player speed** → Mitigation: base frame advancement on movement state with explicit timing constants that can be tuned without changing movement logic.
- **[Risk] Snapshot types become too player-specific** → Mitigation: encode generic sprite draw metadata where practical and keep asset ownership in `Game`.
- **[Risk] Tests currently assert marker geometry** → Mitigation: update snapshot and runtime tests to assert player sprite placement, facing/frame metadata, and asset staging instead of rectangle-marker sizing.

## Migration Plan

1. Extend player-owned state and overworld snapshot types to describe sprite presentation.
2. Replace marker-placement helpers with player sprite placement helpers based on the tile-centered foot pivot.
3. Load and render the player spritesheet in `Game`.
4. Stage the spritesheet into the build asset directory.
5. Update tests to match sprite-backed rendering behavior.

Rollback is straightforward: revert the change to resume marker-based player rendering and remove the added asset staging.

## Open Questions

- Whether the walk animation should advance strictly on elapsed movement time or partially from traveled distance. The initial design assumes time-based advancement gated by movement state because it is simpler and fits the existing update loop.
