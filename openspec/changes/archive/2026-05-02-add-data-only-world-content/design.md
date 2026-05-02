## Context

The current `world-content` boundary is intentionally small: `World` retains chunk content as a flat list of records containing only an id, owning chunk coordinates, and a coarse content type. That shape was enough to prove deterministic chunk content queries, but it does not carry the spatial or appearance data that future gameplay and rendering systems will need.

This change extends the content model while preserving the current module boundaries:

- `World` remains the world-facing query boundary.
- chunk retention remains the source of truth for generated content.
- content stays repo-native and detached from SFML/runtime-shell types.
- the first pass is explicitly non-interactive.

The repo already uses `WorldPosition` and `WorldSize` as canonical world-space types, and existing terrain/render code already treats world positions as tile-centered. The content design should align with those conventions instead of introducing parallel geometry types.

## Goals / Non-Goals

**Goals:**

- Replace the minimal content record with richer data-only structs that describe content identity, spatial placement, footprint, and appearance selection.
- Keep chunk content deterministic for a given world seed, chunk coordinates, and retained chunk metadata.
- Preserve world-facing chunk content queries through `World`.
- Make the new content shape suitable for future rendering and interaction systems without implementing those systems now.

**Non-Goals:**

- Adding interaction logic, triggers, inventories, dialogue, or behavior scripts.
- Publishing content directly through render snapshots in this change.
- Introducing SFML-facing rendering identifiers or asset loading concerns into the world-content model.
- Changing chunk metadata responsibilities beyond what is needed to derive content deterministically.

## Decisions

### Use a chunk container plus per-instance data-only records

The content model will move from a flat `WorldContentRecord` list toward an explicit chunk container such as `ChunkContent` that owns a collection of `ContentInstance` values.

Why:

- chunk ownership is chunk-scoped state and should not be duplicated on every instance.
- the container gives the world-facing query boundary a natural place to grow if future chunk-level content metadata is needed.
- the distinction between chunk-scoped retention and instance-scoped data becomes clearer in both code and specs.

Alternative considered:

- Keep the existing flat vector shape and add more fields to each record. This was rejected because it keeps chunk ownership duplicated per record and makes the content boundary less expressive.

### Reuse existing world-space types for content placement

Content instances will use `WorldPosition` for placement and `WorldSize`-backed footprint data for occupied space.

Why:

- these are already the repo-native world-space types.
- they align with current world and camera conventions.
- reuse avoids introducing duplicate geometry types that represent the same concepts.

Alternative considered:

- Introduce dedicated content geometry types immediately. This was rejected because the current distinction is not yet meaningful enough to justify parallel math types.

### Treat content position as the center of the footprint

Content placement will use a world-space center position rather than a corner anchor.

Why:

- current overworld APIs already expose tile centers as the primary placement convention.
- center anchoring keeps future alignment with tile-based placement straightforward.
- it avoids mixing anchor conventions across world-facing data.

Alternative considered:

- top-left anchoring. This was rejected because it would diverge from existing center-based world queries and force additional translation at call sites.

### Separate semantic type from presentation selection

Each content instance will carry both a semantic `ContentType` and an opaque appearance identifier.

Why:

- semantic type answers what the content is.
- appearance id answers how it should look without coupling the model to concrete rendering assets.
- this keeps room for deterministic visual variation within the same semantic type.

Alternative considered:

- encode appearance solely through `ContentType`. This was rejected because it conflates gameplay meaning with presentation variation and limits future extensibility.

### Keep the first pass explicitly non-interactive

The model will exclude interaction state such as use actions, trigger volumes, scripts, or inventories.

Why:

- the user request is to keep content data-only at first.
- interaction concerns would force premature decisions about gameplay boundaries.
- a clean data-first step is easier to test and easier to extend later.

Alternative considered:

- add placeholder interaction flags now. This was rejected because placeholder gameplay fields tend to solidify accidental contracts before the interaction model is designed.

## Risks / Trade-offs

- **[Broader data model increases API churn]** → Mitigation: keep the change limited to `world-content` and closely related tests, and avoid leaking the new shape into unrelated runtime boundaries.
- **[Choosing center anchoring may constrain future oversized content]** → Mitigation: store footprint explicitly so larger content can still be positioned deterministically around a center anchor.
- **[Opaque appearance ids may be underspecified for later rendering work]** → Mitigation: define them as stable deterministic identifiers now and postpone asset-binding semantics to a later rendering-focused change.
- **[Container-based chunk content may require test rewrites]** → Mitigation: update content-focused tests to compare chunk-level and instance-level values directly rather than relying on the old flat record shape.
