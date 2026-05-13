## Context

`OverworldRuntime` owns three related responsibilities for the active overworld session: one-time session setup, camera framing for the current viewport, and publication of render/debug snapshots. The current implementation couples those responsibilities by having `update()` call `initialize()`, even though `initialize()` already performs a camera update and snapshot refresh before `update()` advances player state and performs the same work again.

The change is small in surface area but performance-sensitive because the duplicate path repeats retention-window updates, visible-tile queries, visible-content queries, and vector rebuilds every frame. Existing tests also establish an important contract: calling `initialize()` alone leaves a render-ready snapshot available to consumers.

## Goals / Non-Goals

**Goals:**
- Remove the duplicate camera and snapshot refresh work from each `OverworldRuntime::update()` call.
- Preserve the current `initialize()` contract of producing an initial render/debug snapshot for the supplied viewport.
- Keep the change narrow to `OverworldRuntime` and focused tests.

**Non-Goals:**
- Redesign the render snapshot data model or move snapshot ownership out of `OverworldRuntime`.
- Change gameplay behavior, debug overlay semantics, or the order of simulation versus rendering publication beyond the duplicate-refresh fix.
- Introduce broader caching or rendering optimizations outside this immediate per-frame win.

## Decisions

### Separate one-time initialization from snapshot publication

`OverworldRuntime` will gain an internal one-time setup path that handles spawning the player and flipping initialization state without also refreshing the frame snapshot.

**Rationale:** `update()` needs the session to exist, but it does not need `initialize()`'s full publish path. Separating those concerns removes the duplicated work at its source.

**Alternatives considered:**
- Keep calling `initialize()` from `update()` and add guards inside `initialize()` to skip some work. Rejected because it preserves mixed responsibilities and makes the call graph harder to reason about.
- Stop refreshing snapshots from `initialize()` entirely. Rejected because existing tests and the current API contract expect `initialize()` to leave the runtime render-ready.

### Keep `initialize()` as the public render-ready entry point

`initialize()` will continue to update the camera and publish render/debug snapshots for the provided viewport after one-time setup.

**Rationale:** This preserves the observable contract already relied on by tests and keeps the runtime usable in flows that need a valid snapshot before the first simulation step.

**Alternatives considered:**
- Make callers invoke `update(0, ...)` after initialization to obtain the first snapshot. Rejected because it weakens the API, complicates call sites, and adds unnecessary coupling to frame-step semantics.

### Make `update()` own the single per-frame publication pass

After applying debug-view state, ensuring one-time setup, updating the player, and recalculating the camera for the current viewport, `update()` will refresh the render/debug snapshots exactly once for the active frame.

**Rationale:** Snapshot publication belongs at the end of the frame update because it should reflect the post-simulation player position, camera frame, and world visibility for that frame.

**Alternatives considered:**
- Refresh once before simulation and once after only when movement occurs. Rejected because it reintroduces duplicate work and creates inconsistent rules across frames.

## Risks / Trade-offs

- **A helper split could accidentally change initialization ordering** → Keep one-time setup minimal and preserve the existing spawn-before-camera-before-refresh sequence for public `initialize()`.
- **A narrow code change could still regress the published snapshot contract** → Retain initialization-focused tests and add coverage that exercises update-driven publication with debug zoom and viewport changes.
- **The spec change encodes a performance-sensitive behavior** → Phrase the requirement around published frame ownership and non-redundant update publication so the contract stays testable without overspecifying internals.
