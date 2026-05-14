## Context

The overworld debug overlay currently renders both "Rendered objects" and "Visible content" even though `OverworldRuntime` populates both values from the same visible generated-content collection. The duplicate labels make the overlay look more diagnostic than it is and create an avoidable maintenance burden across the debug snapshot, overlay-string formatting, and tests.

This change is intentionally narrow. The runtime already computes one stable generated-content metric for the active frame: the count of generated-content entries visible in the camera frame. Although the render shell may redraw a subset of those entries into occlusion masks, that is a different rendering-cost concern and is not part of the current gameplay-owned debug snapshot contract.

## Goals / Non-Goals

**Goals:**
- Make the debug overlay report only generated-content metrics with distinct meaning.
- Align the debug snapshot implementation with the single visible generated-content metric the gameplay-facing snapshot contract already describes.
- Keep the change small and local to the existing runtime snapshot and overlay formatting path.

**Non-Goals:**
- Introduce a new draw-call or pass-specific rendering metric for generated content.
- Redesign the broader debug overlay layout or wording beyond removing the duplicated metric.
- Change render ordering, occlusion behavior, or generated-content visibility rules.

## Decisions

### Collapse the duplicate overlay metric instead of inventing a new rendered-content counter

The overlay will keep the visible generated-content metric and remove the separate rendered generated-content metric. This matches the runtime data that already exists and avoids introducing a second metric whose meaning would be ambiguous.

Alternative considered: distinguish "rendered" as draw calls or entries submitted across all passes. Rejected because it would mix gameplay-visible diagnostics with renderer-internal work, and the current label would still need tighter definition to avoid confusion.

### Remove the unused rendered-generated-content field from the debug snapshot

The gameplay-owned debug snapshot will expose only the values the overlay meaningfully consumes. Dropping the redundant field keeps the snapshot aligned with the current render-snapshots contract and prevents future code from reviving the duplicate metric by accident.

Alternative considered: keep the field but populate it from the same visible-count source with updated wording elsewhere. Rejected because it preserves a misleading API shape and leaves dead semantic weight in the snapshot type.

### Update overlay and tests together

The overlay string contract and snapshot tests will be updated in the same change so the spec, implementation, and coverage all describe the same metric set.

Alternative considered: change implementation first and defer spec or test cleanup. Rejected because this would temporarily leave artifact expectations inconsistent.

## Risks / Trade-offs

- **Risk:** Removing one overlay line may break tests or downstream expectations that rely on the exact debug string. → **Mitigation:** Update the overlay-string assertions and snapshot-facing tests in the same change.
- **Trade-off:** The overlay will expose less apparent detail about generated content. → **Mitigation:** Keep the retained-content and visible-content metrics, which still describe cache scale and active-frame scale separately.
- **Risk:** Future rendering work may genuinely need a pass-specific generated-content metric. → **Mitigation:** Add it later as a separately named renderer metric with explicit semantics instead of reusing the old label.
