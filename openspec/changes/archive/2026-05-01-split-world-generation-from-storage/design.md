## Context

`World` currently combines two concerns: it retains chunk-backed runtime world state and it encodes the deterministic policy that decides what each chunk contains. That coupling was acceptable while terrain only produced tiles and a spawn, but the next world-building slices will need the same deterministic policy surface to drive props, NPC spawns, points of interest, and eventual save/load without making `World` the owner of every generation rule.

The existing module boundaries already treat `Game` as orchestration and keep gameplay-facing behavior in `World`, `Player`, and `Camera`. This change extends that separation one level deeper by keeping `World` as the runtime-facing owner of cached chunk state while moving sampling and generation decisions into dedicated internal collaborators.

Constraints:
- Preserve the current public `World` API and observable runtime behavior.
- Keep deterministic chunk results stable for a given `WorldConfig` and absolute coordinate.
- Avoid introducing plugin-style abstractions or public generation interfaces before there is a second implementation.

## Goals / Non-Goals

**Goals:**
- Separate chunk content derivation from chunk retention and runtime querying.
- Let `World` own cache lifecycle, world-facing lookup APIs, and reuse of retained chunks.
- Centralize biome sampling and terrain-generation policy in internal collaborators that can later expand to props, NPC spawns, POIs, and save data derivation.
- Preserve current deterministic terrain and spawn behavior as the compatibility baseline.

**Non-Goals:**
- Changing the terrain algorithm, biome thresholds, or spawn rules as part of this proposal.
- Exposing a new public generator API in `include/main/`.
- Introducing runtime-swappable generation backends, dependency injection containers, or persistence formats in this slice.

## Decisions

### Use an internal generation-policy collaborator behind `World`

`World` will delegate missing-chunk materialization to a dedicated internal collaborator such as `TerrainGenerator`, supported by lower-level helpers like `BiomeSampler` as needed. The collaborator will accept world configuration plus absolute chunk coordinates and return deterministic generated chunk content.

`World` remains the gameplay-facing module:
- it owns the retained chunk cache,
- it decides when a missing chunk must be generated,
- and it serves tile/render/runtime queries from retained state.

**Why this approach**
- It cleanly separates "when runtime state is needed" from "what deterministic content that state should contain."
- It matches the repo's current bias toward small internal collaborators rather than new public layers.
- It creates a reusable policy surface for future generated systems without changing the `World` consumer contract.

**Alternatives considered**
- **Keep helpers inside `World.cpp`**: simplest mechanically, but it leaves the ownership boundary ambiguous and does not prepare for new generated systems.
- **Move cache ownership into a new storage module too**: possible later, but it makes this slice broader than necessary; the immediate pressure is separating generation policy from runtime storage.
- **Introduce a public abstract generator interface**: premature for a codebase with one concrete overworld generation flow.

### Keep chunk cache entries as generated runtime state, not generation policy state

The retained chunk cache should store the outputs needed by runtime queries and rendering, while sampling noise functions, biome classification rules, and similar policy code stay outside the cache owner. `World` can therefore expand cached chunk payloads over time without also becoming the author of how every field is derived.

**Why this approach**
- Future props/NPC/POI data likely belong to chunk state, but their derivation rules should remain centralized in generation policy.
- It keeps save/load evolution clearer: serialized world state is distinct from the deterministic logic that can reconstruct or enrich it.

**Alternatives considered**
- **Let `World` continue owning both cache schema and generation rules**: keeps fewer files today, but scales poorly as generated content types multiply.

### Preserve deterministic behavior as the first-class compatibility target

This change is structural. For identical inputs, the resulting chunk contents, tile traversability, and spawn outcomes should remain unchanged.

**Why this approach**
- Existing world-generation behavior is already the runtime contract.
- Structural refactors are safer and easier to review when they avoid gameplay changes.

## Risks / Trade-offs

- **Generator boundary is too narrow for future needs** → Keep the generated result shape chunk-oriented and extensible so more generated content can be added without re-merging policy into `World`.
- **Policy/cache split adds indirection to a small codebase** → Limit the change to one internal collaborator boundary and reuse existing data types where possible.
- **Behavior drift during extraction** → Treat current deterministic terrain and spawn behavior as compatibility requirements and preserve the same coordinate-driven outputs.

## Migration Plan

No runtime migration is required. Implementation should introduce the internal generation collaborator, route `World`'s cache misses through it, update relevant tests/specs, and preserve the existing public module surface. If follow-up work later adds save/load, that work can build on this split without revisiting the boundary.

## Open Questions

- None for this slice. The intended direction is to keep the new generation-policy boundary internal until a real external consumer appears.
