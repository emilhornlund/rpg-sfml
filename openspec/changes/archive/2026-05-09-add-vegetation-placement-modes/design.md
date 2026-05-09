## Context

The current vegetation pipeline loads `family`, `placeOn`, and biome weights from staged classification data, but runtime spawn behavior still collapses prototype semantics into a binary split: `tree` prototypes use a sparse tree pass and every other prototype uses a dense shrub-style pass. That makes forest debris tuning hard because props such as stumps and logs compete inside the same broad non-tree pass as bushes and water plants.

The classification now includes explicit `placementMode` values such as `tree_sparse`, `ground_dense`, and `prop_sparse`. This change needs to carry that metadata through loading and make world content generation branch on the declared mode instead of inferring spawn behavior from `family`, tags, or prototype names.

## Goals / Non-Goals

**Goals:**
- Preserve `placementMode` on loaded vegetation prototypes and use it as the source of truth for spawn behavior.
- Preserve deterministic content identities and stable biome-driven prototype selection.
- Produce a clear density hierarchy in forests: many trees, some ground cover, occasional props.
- Keep biome weights meaningful as weights within each placement mode's candidate pool.

**Non-Goals:**
- Reinterpret biome weights as absolute global probabilities.
- Redesign rendering or content record types around new public gameplay APIs.
- Introduce arbitrary per-prototype hardcoded exceptions in runtime placement code.

## Decisions

### Add explicit placement-mode metadata to loaded vegetation prototypes
`VegetationPrototype` should carry a `placementMode` field loaded from anchor metadata. This keeps the model explicit and allows runtime placement to ask the prototype how it should be sampled instead of guessing from `family`.

Alternative considered: continue inferring spawn behavior from `family` or from tags. Rejected because it recreates the same ambiguity that made stump tuning brittle.

### Keep render/content type separate from placement mode
`ContentType` should continue to describe the published content category used by the runtime and renderer, while `placementMode` decides which placement pass samples a prototype. This avoids overloading one field to mean both "what is this object?" and "how should it spawn?"

Alternative considered: replace `ContentType` with `placementMode`. Rejected because sparse props such as logs and stumps still belong with non-tree object presentation even though their spawn behavior differs from dense ground cover.

### Implement three placement paths with different densities
The runtime should map:
- `tree_sparse` to the existing sparse tree-style pass,
- `ground_dense` to the existing dense shrub-style pass,
- `prop_sparse` to a new sparse prop pass that is less frequent than trees.

This preserves existing behavior where it already matches intent while creating an explicit debris/prop lane for stumps and logs.

Alternative considered: reuse the tree sparse pass unchanged for props. Rejected because the desired hierarchy calls for many trees but only occasional props, so props need a sparser cadence than trees.

### Keep biome weights local to a mode-specific pool
For a given placement pass, the runtime should first determine whether that mode spawns at the candidate location, then use biome weights only to choose among prototypes eligible for that mode and terrain class. This keeps the existing weight semantics but narrows them to comparable prototypes.

Alternative considered: multiply mode spawn chance directly by biome weight. Rejected because it would change the meaning of current biome data and make tuning less predictable.

## Risks / Trade-offs

- **Introducing `placementMode` without updating all staged anchors could leave prototypes without valid spawn behavior** → Require loader validation or a safe explicit default policy during rollout, and cover representative anchors in tests.
- **A new sparse prop pass could still produce too many or too few props if tuned poorly** → Start with conservative spacing and verify aggregate forest composition in tests rather than exact counts.
- **Keeping `ContentType` and `placementMode` separate adds another concept to the model** → Document the distinction clearly in specs and tests: one field drives spawn behavior, the other drives published content semantics.

## Migration Plan

- Update staged vegetation anchors to include `placementMode`.
- Preserve `placementMode` in vegetation metadata loading.
- Switch runtime placement to build and sample pools by mode.
- Update focused tests for asset loading and vegetation density behavior.

## Open Questions

- What exact grid size and chance curve should `prop_sparse` use relative to `tree_sparse`?
- Should water plants remain entirely under `ground_dense`, or should some future water props use additional modes?
