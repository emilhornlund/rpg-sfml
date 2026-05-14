## Context

`World` retains generated chunk tiles, metadata, and content in its internal chunk cache. The current world-facing metadata and content accessors return those retained payloads by value, which means read-only inspection performs a copy even when the caller only needs to traverse existing retained data.

The copy cost is trivial for current metadata and acceptable for small content payloads, but `ChunkContent` already owns a `std::vector<ContentInstance>` and each instance includes additional payload such as `prototypeId`. As chunk density and content richness grow, repeated inspection-only calls become an avoidable cost. At the same time, the retained chunk cache is not permanently stable because `updateRetentionWindow()` can evict chunks.

## Goals / Non-Goals

**Goals:**
- Add a world-facing borrowed inspection path for retained chunk metadata and chunk content.
- Make the lifetime of borrowed results explicit and consistent with retained chunk eviction.
- Preserve an owned snapshot path for callers that need to keep data beyond the borrowed lifetime window.
- Keep the change narrow and aligned with the current `World` chunk-cache design.

**Non-Goals:**
- Rework chunk retention, generation, or eviction policy.
- Introduce new gameplay-facing content semantics.
- Add new storage containers or a broad visitor framework unless the chosen API cannot satisfy the lifetime and performance goals.

## Decisions

### Decision: Add borrowed accessor APIs and keep snapshot access available

`World` will expose borrowed inspection APIs for retained chunk metadata and chunk content so inspection-only callers can avoid copying retained payloads. The borrowed API can be expressed either as explicit `const` reference accessors or as a lightweight view type, but the contract is the same: callers receive read-only access to retained chunk data owned by `World`.

The existing by-value behavior will remain available as an owned snapshot path so tests and future callers that need data independent of chunk eviction can keep that behavior intentionally.

**Why this over replacing the current accessors outright?**
- It avoids forcing every caller into a borrow-sensitive lifetime model.
- It keeps the migration incremental.
- It makes ownership explicit at the API boundary instead of overloading a single accessor with conflicting expectations.

**Alternatives considered:**
- **Replace the current accessors with `const&` accessors only:** simplest mechanically, but it turns all existing and future call sites into lifetime-sensitive code with no owned alternative.
- **Visitor-style enumeration only:** avoids exposing references but adds a heavier API shape than the current codebase appears to need for straightforward chunk inspection.

### Decision: Tie borrowed result validity to retained chunk lifetime

Borrowed metadata and content results are valid only while the owning `World` instance still retains the addressed chunk and remains alive. Any later operation that can evict retained chunks, especially `updateRetentionWindow()`, ends that validity window for affected chunks.

This contract matches the current architecture: borrowed access reflects retained cache state instead of promising stable ownership independent of eviction.

**Alternatives considered:**
- **Promise stable borrows for the full `World` lifetime:** not compatible with eviction unless chunk retention is redesigned.
- **Hide lifetime rules and rely on caller intuition:** too risky for a performance-motivated API change.

### Decision: Keep the borrowed API aligned for metadata and content

Even though metadata copies are much cheaper than content copies today, metadata and content inspection will follow the same borrowed-access pattern. That keeps the `World` query surface internally coherent and prevents callers from having to remember two ownership models for neighboring chunk inspection APIs.

**Alternatives considered:**
- **Optimize content only:** valid in isolation, but it creates an uneven API boundary and likely invites a second API change later when metadata grows or callers expect symmetry.

## Risks / Trade-offs

- **Borrowed results can dangle after chunk eviction** → Document invalidation rules in the API and keep an owned snapshot path for callers that need to retain data across retention-window updates.
- **Parallel owned and borrowed APIs can feel redundant** → Use naming and documentation that make the ownership difference explicit.
- **Future API shape may outgrow plain references** → If additional read-only chunk inspection surfaces appear, the borrowed contract can be carried forward into a dedicated view type without changing the core lifetime model.

## Migration Plan

1. Add borrowed chunk inspection APIs and document their lifetime guarantees.
2. Retain or add explicit snapshot-producing accessors for owned copies.
3. Update tests to cover both borrowed inspection behavior and owned snapshot behavior.
4. Migrate internal or future callers to the borrowed path only when they do not need ownership beyond the retained lifetime window.

## Open Questions

- Should the borrowed content API be a direct `const ChunkContent&` accessor or a narrower view type that exposes chunk coordinates and a read-only instance range?
- Should the existing `getChunkMetadata()` / `getChunkContent()` names remain the snapshot path, with borrowed access introduced under new names, or should the borrowed path take the primary names and snapshots move to explicit copy-oriented names?
