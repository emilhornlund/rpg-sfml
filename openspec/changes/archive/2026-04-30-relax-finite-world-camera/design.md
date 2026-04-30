## Context

The current overworld slice keeps a finite generated world behind `World`, but the camera boundary is still coupled to that finite shape. `Camera::update()` takes `World` so it can read `getWorldSize()` and clamp the frame center and size, while `Game` renders whatever `World::getVisibleTiles(frame)` returns inside that clamped frame. This change is intentionally narrower than an infinite-world rewrite: the goal is to let camera behavior stop depending on finite extents and to stop requiring finite-world APIs in camera-facing flows, while preserving the existing finite slice generation, collision, and render traversal model.

## Goals / Non-Goals

**Goals:**
- Let camera framing follow the focus position directly without clamping to generated world bounds.
- Decouple the camera API from `World` so camera updates depend only on focus position and viewport size.
- Keep the finite overworld slice internally intact for generation, traversability, spawn selection, and visible terrain enumeration.
- Allow rendering near map edges to show non-terrain background outside the generated world instead of forcing the camera to stay within bounds.
- Clarify which world APIs remain necessary once camera framing no longer depends on world size.

**Non-Goals:**
- Introduce infinite terrain generation, streaming, or lazy chunk creation outside the configured finite slice.
- Remove finite sizing inputs from `WorldConfig` or change how the current overworld slice is generated.
- Change player movement rules, spawn placement semantics, or collision behavior at world edges.
- Add synthetic "void tiles" outside the world; rendering empty background is sufficient.

## Decisions

### Decision: Make camera updates world-independent
- `Camera::update()` should accept only the focus position and desired viewport size, then store a frame centered on that focus without consulting `World`.
- This removes the camera's dependency on finite-world extents and makes camera behavior a pure framing concern.
- Alternative considered: keep the `World` parameter and add a flag to disable clamping. This preserves backward compatibility, but it keeps finite-world knowledge in the camera boundary and muddies the API contract.

### Decision: Preserve finite world behavior behind `World`
- `World` should remain responsible for finite-slice generation, traversability checks, and visible terrain enumeration within the configured tile rectangle.
- This keeps A and B scoped to camera and API cleanup rather than turning the change into unbounded world generation.
- Alternative considered: extend `World` to generate chunks on demand for any absolute coordinate. The chunk coordinate math already supports this, but it is a larger behavioral shift that would require new terrain, spawn, and movement requirements.

### Decision: Render empty space outside the generated slice
- When an unclamped camera frame extends beyond the generated overworld, rendering should still enumerate only generated terrain tiles that intersect the frame and leave the rest of the view as the existing background clear color.
- This preserves a simple rendering model and avoids inventing new tile types or world data for out-of-bounds space.
- Alternative considered: have `World` emit placeholder tiles for space outside the map. That would blur the boundary between generated terrain and presentation-only background.

### Decision: Narrow finite-world API expectations rather than forcing full API removal in one step
- The spec should stop requiring camera-facing flows to depend on world-size accessors, but implementation may either remove those accessors outright or retain them for non-camera uses if they still serve tests or world-facing logic.
- This keeps the change focused on behavior and ownership while leaving room for a precise API cleanup during implementation.
- Alternative considered: require full removal of all world dimension accessors at the spec level. That is more rigid than necessary for the A+B goal and would turn a behavior change into an all-or-nothing API migration.

## Risks / Trade-offs

- **Unclamped framing can reveal large background regions near edges** → Preserve the current world-only visible terrain query and rely on the existing background clear color for non-terrain space.
- **Retaining some finite-world accessors could leave the API cleanup incomplete** → Make the spec explicit that camera-facing code must not require them, even if some accessors remain for world-owned behavior.
- **Tests written around clamped camera centers will no longer hold** → Replace them with focus-following and edge-rendering scenarios that verify the new contract directly.
- **This change could be mistaken for infinite-world groundwork** → Keep the proposal and design explicit that chunk generation, movement, and world traversal remain finite in this change.

## Migration Plan

1. Update the overworld vertical slice spec to remove clamped-camera requirements and to allow rendering beyond generated terrain near edges.
2. Implement the camera API change and adapt runtime call sites to pass only focus position and viewport size.
3. Trim or de-emphasize finite-world APIs that are no longer needed for camera-facing flows.
4. Update tests to validate direct focus-following camera behavior and world-edge rendering with unclamped frames.

Rollback is straightforward: restore the previous camera contract and clamped-framing expectations in the runtime and tests.

## Open Questions

- Whether `World::getWorldSize()` and related dimension accessors should be removed immediately or simply stop being required by camera-facing code.
- Whether any non-camera callers still need explicit world dimension access after the change, or whether tile-size and coordinate conversion APIs are sufficient.
