## Context

`World` already retains deterministic chunk content together with tiles and chunk metadata, and it exposes chunk-addressed content queries through the gameplay boundary. `OverworldRuntime` currently stops short of that boundary: it derives the active camera frame, collects visible terrain tiles, converts player state into a render marker, and publishes only those values through `OverworldRenderSnapshot`.

That split leaves the SFML shell with no runtime-owned path for generated content. If the game shell needs to draw chunk content, it would either have to query `World` directly or reconstruct world/chunk visibility rules outside the gameplay boundary. The existing architecture points to a cleaner seam: keep visibility and retention behind `World`, and let `OverworldRuntime` assemble one render-ready snapshot containing all visible overworld presentation data.

## Goals / Non-Goals

**Goals:**
- Let the overworld runtime publish visible generated content through the same render snapshot boundary that already carries terrain and player state.
- Preserve the current ownership split where `World` owns retained chunk data and visibility-relevant world queries, while `OverworldRuntime` owns per-frame snapshot assembly.
- Add render-facing generated-content data that is rich enough for drawing without forcing `Game.cpp` to reinterpret `ChunkContent` records or query gameplay modules directly.
- Keep the first pass deterministic and data-oriented, using the existing generated content model and current shell rendering flow.

**Non-Goals:**
- Adding gameplay interaction systems for generated content.
- Redesigning chunk generation, content generation, or chunk retention ownership.
- Introducing SFML-specific types into gameplay-facing world or snapshot data.
- Defining a full asset pipeline for every generated content type beyond the render-facing identifiers needed for the first rendering pass.

## Decisions

### `World` owns visible generated-content enumeration

Generated content visibility should be exposed through a world-facing query such as `getVisibleContent(const ViewFrame&)`, parallel to `getVisibleTiles`.

Why:
- `World` already owns chunk retention, chunk-coordinate math, and the camera-frame-to-visible-region traversal used for terrain.
- visible content should be derived from retained chunk content without making the runtime shell or `OverworldRuntime` duplicate chunk enumeration and deduplication logic.
- keeping this query inside `World` preserves the gameplay module boundary: callers still consume world-owned queries rather than storage details.

Alternatives considered:
- Enumerate visible content inside `OverworldRuntime` by querying chunk content directly: rejected because it duplicates world visibility logic and makes runtime assembly responsible for chunk traversal details.
- Let `Game.cpp` ask `World` for visible content while rendering: rejected because it breaks the render snapshot boundary and pushes gameplay interpretation back into the shell.

### The render snapshot gets a dedicated generated-content collection

Generated content should be published as its own render-facing entry collection in `OverworldRenderSnapshot`, rather than being forced into the existing player-oriented marker type.

Why:
- `OverworldRenderMarker` is currently actor-shaped: it carries facing and animation selectors that only make sense for the player-style sprite path.
- generated content already has a different source model (`ContentType`, footprint, opaque appearance id), so a dedicated snapshot entry preserves meaning instead of stuffing unrelated fields into one catch-all struct.
- the resulting snapshot remains easier to consume because terrain, actors, and world props each have a clear render-facing shape.

Alternatives considered:
- Reuse `OverworldRenderMarker` for generated content: rejected because it couples world props to player-only presentation fields and muddies the boundary.
- Publish raw `ContentInstance` values directly in the snapshot: rejected because the snapshot should carry render-facing geometry and identifiers, not raw retained world-content records that the shell must reinterpret.

### `OverworldRuntime` remains the frame assembler

`OverworldRuntime::refreshRenderSnapshot()` should continue to be the single place where the active camera frame is turned into a render snapshot, now by merging visible terrain, player presentation, and visible generated content.

Why:
- this keeps one clear transition from gameplay state to frame data.
- it matches the current runtime contract and test surface.
- the shell remains a thin renderer that consumes the snapshot rather than orchestrating gameplay data sources.

Alternatives considered:
- Split snapshot assembly across `World` and `OverworldRuntime`: rejected because it blurs ownership of the final per-frame boundary.

### Visibility should be frame-based and footprint-aware

Generated content visibility should be determined against the active camera frame using world-space placement and footprint information, rather than only checking whether a content center point falls inside the frame.

Why:
- the content model already carries footprint size.
- larger content should remain visible when part of its footprint intersects the active frame.
- footprint-aware visibility prevents avoidable popping behavior at view edges.

Alternatives considered:
- Center-point visibility only: rejected because it is simpler but produces incorrect edge behavior for multi-tile content.

### First-pass rendering keeps existing draw-order expectations

The initial rendering contract should place generated content in the world layer between terrain and player presentation so world props participate in the visible frame without obscuring the player's final presentation contract.

Why:
- the current vertical-slice expectations already distinguish terrain and player layers.
- generated world props visually belong in the map rather than the actor overlay layer.
- this creates a predictable place for future layering refinement without delaying the first pass.

Alternatives considered:
- Draw generated content above the player: rejected because it risks obscuring the player by default.
- Leave layering unspecified: rejected because rendering order is user-visible behavior and should be testable.

## Risks / Trade-offs

- **[Visible-content queries may partially duplicate visible-tile frame traversal]** → Mitigation: reuse the same frame/chunk bound calculations or extract a shared helper so terrain and content stay aligned.
- **[Introducing another snapshot entry type expands the public runtime contract]** → Mitigation: keep the new entry narrowly focused on world-space geometry plus opaque presentation identifiers.
- **[Footprint-aware visibility adds slightly more logic than center-point filtering]** → Mitigation: constrain the first pass to simple rectangle/frame intersection based on existing world-space types.
- **[Initial render identifiers may be too coarse for future art variation]** → Mitigation: preserve opaque content appearance ids now so later rendering work can refine asset mapping without changing world generation rules.

## Migration Plan

1. Add world-facing visible-content enumeration built on retained chunk content and active frame bounds.
2. Extend `OverworldRenderSnapshot` with a generated-content render entry type and collection.
3. Update `OverworldRuntime` snapshot assembly to merge terrain, player, and visible generated content into the published frame.
4. Update shell rendering and tests to consume the new snapshot data and render generated content in the agreed layer order.

Rollback is straightforward: remove the visible generated-content snapshot path and return generated content to a world-only retained query surface.

## Open Questions

- Should the first-pass content rendering reuse existing terrain/player art assets as placeholders, or is a separate content visual mapping expected in the same change?
- Do we want the world-facing visible-content query to return only render-relevant content, or all visible retained content even if some types are not yet drawn?
