## Context

The current overworld generator produces deterministic results, but it classifies each interior tile from a mostly independent hashed value. That keeps tests and seeds stable while making the terrain read as random tile noise rather than connected geography.

This change keeps the existing `World` API, tile categories, and deterministic-seed contract, but changes how tile categories are derived. The implementation should stay inside the existing world-generation collaborator boundary so `World` continues to own state and runtime queries while terrain construction remains isolated in dedicated helpers.

## Goals / Non-Goals

**Goals:**
- Produce terrain from spatially coherent environmental signals instead of one-off per-tile thresholds.
- Use elevation as the primary landform signal and moisture as the biome-cover signal.
- Preserve deterministic generation, world dimensions, tile lookup, traversability rules, rendering inputs, and spawn selection.
- Keep the change compatible with the current four tile categories: water, sand, grass, and forest.
- Expand tests to verify signal-driven biome mapping and stable generation behavior.

**Non-Goals:**
- Adding new tile categories, decorations, rivers, roads, or structure placement.
- Changing the public `World` interface or the top-level runtime architecture.
- Introducing external noise-generation dependencies or a new content pipeline.
- Solving long-range gameplay concerns such as chunk streaming or infinite worlds.

## Decisions

### Decision: Generate two independent terrain signals

The generator will derive normalized elevation and moisture values from deterministic coordinate-based functions seeded from the world configuration. Elevation and moisture stay logically separate so landform shape and vegetation cover can evolve independently.

**Why this decision:** A single scalar still tends to collapse the map into threshold bands. Two signals support coherent coastlines and inland biome variation with the tile set that already exists.

**Alternatives considered:**
- **Single improved noise field:** simpler, but still couples shoreline shape and vegetation density too tightly.
- **Hard-coded region painting:** can look coherent, but is less reusable and less naturally deterministic across sizes and seeds.

### Decision: Build elevation from layered frequencies plus edge falloff

Elevation should combine broad continent-scale variation with smaller regional detail, then apply a border falloff so the existing expectation of water around the outer edge remains true without relying on a hard border-only rule.

**Why this decision:** Large-scale variation creates landmasses; smaller variation avoids flat blobs; edge falloff preserves a finite-map feel and keeps border-water tests compatible with the world slice.

**Alternatives considered:**
- **No edge falloff:** risks land touching the border too often and weakens the current finite-slice framing.
- **Keep hard border water only:** preserves correctness, but coastlines still feel abrupt near the edge.

### Decision: Map tile types through a 2D biome lookup

Tile classification will treat elevation as the first discriminator and moisture as the second:
- below sea threshold -> water
- near sea threshold -> sand
- inland and wetter -> forest
- inland and drier -> grass

**Why this decision:** A lookup driven by both signals makes tile types read as environmental outcomes instead of arbitrary random buckets. Sand becomes a shoreline band instead of isolated spots, and forest distribution becomes a climate decision instead of a lucky high roll.

**Alternatives considered:**
- **Elevation-only mapping:** gives coherent coasts but cannot distinguish forest from grass meaningfully.
- **Rule chain on hashed tile categories:** preserves today’s implementation style but does not solve the spatial-noise problem.

### Decision: Keep spawn selection centered and traversable

Spawn selection will continue scanning outward from the center for a traversable tile after generation completes.

**Why this decision:** The current spawn behavior already satisfies the gameplay contract and remains compatible with signal-driven terrain. Keeping it stable narrows the scope of change.

**Alternatives considered:**
- **Biome-preferred spawn heuristics:** potentially nicer spawns, but adds behavior changes beyond the core terrain-generation goal.

### Decision: Verify structure through behavior-focused tests

Tests should continue checking determinism, borders, traversability, and spawn validity, and add expectations that signal-driven generation produces shoreline sand and inland biome differentiation in a deterministic way.

**Why this decision:** The change is behavioral, so the contract should be locked down through externally visible outcomes rather than implementation-specific helpers.

**Alternatives considered:**
- **Testing raw intermediate signals:** useful during implementation, but too coupled to internal formulas for the long-term spec surface.

## Risks / Trade-offs

- **Threshold tuning may still create bland maps** -> Mitigate by choosing layered signal weights that separate oceans, coastlines, and inland regions clearly across multiple seeds.
- **Too much edge falloff can shrink playable landmass** -> Mitigate by keeping the falloff gradual and preserving a robust inland traversable area.
- **Behavior-focused tests may be overly brittle if they expect exact biome counts** -> Mitigate by testing deterministic properties and structural relationships instead of full-map snapshots.
- **Keeping only four tile categories limits biome richness** -> Accept for now; the goal is better coherence with the existing tile set, not a larger biome catalog.

## Migration Plan

1. Replace the existing per-tile threshold classifier with deterministic layered signal evaluation inside the terrain generator.
2. Map elevation and moisture combinations to the existing tile types.
3. Update terrain-generation tests to assert the revised behavioral guarantees.
4. Keep rendering and world-facing code unchanged except where they rely on the new tile layout behavior.

Rollback is straightforward: revert the generator and test updates to restore the current hash-threshold classification.

## Open Questions

- Whether shoreline sand should depend solely on elevation proximity to sea level or also consider adjacency to water.
- Whether the edge-water guarantee should remain absolute at the outermost border or become fully emergent from the falloff function while preserving current observable behavior.
