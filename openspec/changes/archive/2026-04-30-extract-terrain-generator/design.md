## Context

`World` currently mixes two responsibilities in `src/main/World.cpp`: owning the generated overworld state used by gameplay systems and encoding the concrete rules that generate that state. The current implementation is still small, but the terrain-classification and spawn-selection helpers already form a distinct concern with their own deterministic behavior.

The repository already treats `Game` as an orchestration boundary and uses dedicated gameplay-facing modules (`World`, `Player`, `Camera`) to keep domain logic from collecting in the runtime coordinator. This change extends the same idea inside the world module: `World` remains the gameplay-facing boundary, but the terrain-construction details move into a dedicated collaborator so future generation changes do not bloat `World`.

Constraints:
- Preserve the current `World` public API and consumer behavior.
- Keep deterministic generation stable for a given `WorldConfig`.
- Stay consistent with the existing small-module layout under `src/main/`.

## Goals / Non-Goals

**Goals:**
- Move terrain generation rules and spawn selection out of `World.cpp` into a dedicated helper/module.
- Keep `World` responsible for storing generated state and answering world-facing queries.
- Preserve existing deterministic tile layout, traversability, dimensions, and spawn behavior.
- Make future generation-rule changes local to the generator module instead of the `World` implementation.

**Non-Goals:**
- Changing the generated overworld rules or introducing new terrain types.
- Adding a new public runtime API for terrain generation.
- Introducing plugin-style generators, strategy registration, or runtime-swappable generation systems.

## Decisions

### Use a private world-generation helper with a generated-data result

The implementation will introduce a dedicated helper/module under `src/main/` that accepts `WorldConfig` and returns the generated tile grid plus the chosen spawn tile.

This keeps the extraction narrow:
- `World` calls the helper during construction.
- `World` stores the returned data in its existing state container.
- Callers continue using `World` getters and traversal queries with no public API changes.

**Why this approach**
- It isolates procedural rules behind a single collaborator without widening the public boundary.
- It keeps the deterministic generation contract centralized and easier to evolve.
- It matches the repository’s preference for dedicated gameplay-facing modules and small helpers over pushing more logic into `Game` or large architectural layers.

**Alternatives considered**
- **Keep anonymous-namespace helpers inside `World.cpp`**: simplest today, but it leaves the growth problem unaddressed and keeps future generation changes coupled to `World`.
- **Expose a public generator type in `include/main/`**: more directly testable, but it adds public surface area before the project has a demonstrated need for external generator consumers.
- **Add a polymorphic generation interface**: overly abstract for the current codebase size and would introduce indirection without a concrete second implementation.

### Keep spawn selection inside the generator flow

The helper/module will own both terrain layout generation and the deterministic spawn-tile selection derived from that layout.

**Why this approach**
- Spawn selection is part of world construction, not runtime world querying.
- The generated tile grid and the chosen spawn tile form one deterministic output of the same configuration.
- It prevents `World` from retaining part of the generation algorithm after the extraction.

**Alternatives considered**
- **Leave spawn selection in `World`**: workable, but it splits construction logic across two places and weakens the boundary this change is trying to create.

### Keep the existing behavior as the compatibility target

The helper/module should preserve the current world-generation outcomes for identical inputs.

**Why this approach**
- Existing tests already define the observable contract for deterministic generation, configured dimensions, valid spawn selection, and traversability.
- This proposal is about improving structure, not changing gameplay behavior.

## Risks / Trade-offs

- **Generator remains internal and less directly reusable** → Mitigation: keep the helper result shape simple so it can be promoted later if a public boundary becomes necessary.
- **Boundary becomes too granular for a small codebase** → Mitigation: keep the extraction limited to terrain construction and avoid introducing interfaces or additional layers.
- **Behavior drift during refactor** → Mitigation: preserve the existing algorithms and use current overworld tests as the compatibility guardrail.

## Migration Plan

No runtime migration is required. Implementation should proceed by introducing the helper/module, wiring `World` construction through it, updating build files for the new compilation units, and preserving the existing tests.

## Open Questions

- None currently. The preferred shape is an internal helper/module rather than a new public API.
