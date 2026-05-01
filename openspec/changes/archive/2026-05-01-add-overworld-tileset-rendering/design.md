## Context

The current overworld vertical slice already separates gameplay-owned tile discovery from SFML drawing. `World` resolves visible terrain as `TileType` values, `OverworldRuntime` packages that data into render snapshots, and `Game.cpp` is the only layer that turns those entries into SFML draw calls. Terrain is currently drawn with `sf::RectangleShape` instances and hard-coded colors.

This change introduces the first asset-backed terrain presentation step without changing the gameplay-side snapshot contract. It also changes the overworld runtime tile size baseline from 32 world units to 16 so the simulation and rendered tile geometry match the intended future 16x16 tileset scale. The repo does not yet have a general asset pipeline, so the design also needs a narrow runtime resource strategy for making a tileset image available to the executable.

## Goals / Non-Goals

**Goals:**
- Replace flat-color terrain rectangle rendering with tileset-backed terrain drawing.
- Reduce overworld tile geometry from 32x32 world units to 16x16 world units.
- Keep the existing gameplay boundary intact so `World` and `OverworldRuntime` remain unaware of SFML textures and atlas details.
- Start with a minimal 2x2 terrain atlas that covers the existing four tile categories: grass, sand, water, and forest.
- Add narrowly scoped runtime asset wiring so the executable can load the terrain tileset reliably.

**Non-Goals:**
- Changing world generation, chunk visibility traversal, or render snapshot ownership.
- Introducing animated tiles, multi-layer tilemaps, autotiling, or transition tiles.
- Moving player marker rendering onto the same asset pipeline in this change.
- Defining a generic asset manager for all future game resources.

## Decisions

### Keep `TileType` as the render-facing terrain identifier

The render snapshot already exposes `TileType` per visible tile, which is sufficient for the first tileset-backed terrain pass. The game shell will continue to receive tile geometry plus `TileType` and will translate that identifier into a tileset cell during drawing.

This keeps gameplay modules decoupled from SFML and avoids expanding the snapshot contract to carry texture rectangles or asset identifiers prematurely.

**Alternatives considered:**
- Add atlas coordinates to `OverworldRenderTile`: rejected because it would push rendering-specific presentation details into gameplay-owned snapshot assembly.
- Add SFML texture metadata to `World` or `OverworldRuntime`: rejected because it breaks the current runtime boundary and would make tests more rendering-backend-aware.

### Change the overworld tile size baseline to 16 world units

The first terrain tileset is meant to establish the project’s future tileset scale, so the overworld should stop treating a tile as 32 world units and instead use 16 world units as the runtime tile size baseline. That keeps world-space tile geometry, snapshot geometry, movement steps, and tileset cell size aligned around one unit system.

This is a behavior change beyond just swapping a texture into the renderer, so the implementation should update tile-size-driven geometry and tests accordingly rather than scaling a 16px tileset into the old 32-unit layout.

**Alternatives considered:**
- Keep world tiles at 32 units and only use 16px source art: rejected because it preserves the old geometry and delays the intentional move to a 16-unit tile grid.
- Make tile size configurable as part of this change: rejected because the immediate need is to establish a single new baseline, not introduce broader runtime configuration.

### Store terrain texture ownership in `Game::Impl`

`Game` already owns SFML-facing window and drawing responsibilities. The terrain tileset texture belongs in that shell layer and should be loaded once alongside the render window rather than reloaded per frame or pushed into gameplay code.

**Alternatives considered:**
- Load the texture in a global helper: rejected because lifetime is better expressed through the existing runtime shell implementation object.
- Load the texture inside `OverworldRuntime`: rejected because that module is intentionally SFML-free.

### Use a checked-in minimal tileset asset with fixed atlas mapping

The first atlas should be a simple checked-in image containing exactly four equally sized 16x16 cells in a 2x2 layout. `TileType` values map deterministically to fixed cells, which keeps drawing logic straightforward and makes the initial asset easy to review.

A fixed first mapping also creates a stable stepping stone toward richer art later without forcing a broader data-driven tileset system now.

**Alternatives considered:**
- Continue with generated solid-color rectangles: rejected because it does not move the renderer toward asset-backed tiles.
- Add a configurable data file for tile mappings immediately: rejected because it adds infrastructure before there is more than one tiny atlas to describe.

### Add narrow executable resource staging rather than a general asset pipeline

The runtime needs a dependable way to find the tileset image. The narrowest change is to stage the terrain tileset alongside the executable or in a known relative resource directory as part of the executable build.

This keeps the change self-contained and avoids introducing a more general resource-management system before the project has multiple asset types that need one.

**Alternatives considered:**
- Hard-code an absolute or source-tree asset path: rejected because it is brittle across build directories and environments.
- Build a generic asset manager now: rejected because it is broader than the needs of this first terrain atlas step.

## Risks / Trade-offs

- **[Single-atlas mapping may age quickly]** -> Keep the initial mapping localized to the render shell so a later data-driven or richer tileset system can replace it without changing gameplay-facing modules.
- **[New runtime asset dependency can break local runs if staging is wrong]** -> Wire the tileset into the executable build explicitly and keep the asset path convention narrow and testable.
- **[Smaller world tiles affect movement feel and view density]** -> Update tile-size-driven geometry and tests together so the new 16-unit baseline is applied consistently across traversal, snapshots, and rendering.
- **[Tests currently focus on boundaries, not asset-backed terrain drawing]** -> Extend tests around rendering boundary expectations and asset availability without coupling gameplay tests to SFML texture internals.
