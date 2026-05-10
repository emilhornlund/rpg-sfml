## Context

The overworld renderer already draws terrain and then renders vegetation content and the player through a single deterministic y-sorted object queue. That ordering is valuable and should stay intact. The problem is readability: when dense foreground objects sort in front of the player, the player can disappear completely.

The corrected goal is not canopy splitting or staged tree presentation. The desired effect is to keep the normal world draw order, then reveal only the hidden portion of the player as a faint silhouette wherever front occluders cover player pixels. The effect should work for any front object represented in the existing y-sorted object pass, not only tree families.

## Goals / Non-Goals

**Goals:**
- Preserve the existing global y-sorted overworld object presentation.
- Show a partial player silhouette only where front objects actually occlude the player.
- Keep the visible portion of the player sprite unchanged.
- Avoid new artwork and avoid gameplay-side object-family special cases.

**Non-Goals:**
- Do not change world generation, placement, chunk visibility, or object ordering semantics.
- Do not implement tree-only rules such as trunk/canopy staging.
- Do not fade whole objects when the player overlaps them.
- Do not introduce new external dependencies or asset formats.

## Decisions

### Keep silhouette rendering in the SFML shell

The silhouette effect will be implemented in `Game.cpp` and related shell-side render helpers rather than changing gameplay-owned snapshot types.

**Why:** The current snapshot already publishes the player marker and the generated-content entries needed to construct the same y-sorted render queue the shell already uses. The new effect is a presentation concern layered on top of that existing draw order rather than new gameplay data.

**Alternatives considered:**
- **Add dedicated silhouette metadata to the render snapshot:** rejected because the shell can already derive occluding front objects from the existing y-sorted render queue.
- **Move occlusion logic into gameplay code:** rejected because the effect is SFML-specific compositing, not gameplay state.

### Derive occluders from objects that sort after the player

The shell will keep the current y-sorted object queue and identify front occluders as generated-content entries that render after the player marker in that queue.

**Why:** This preserves the current world-depth semantics. The silhouette should represent “what is currently in front of the player,” not a separate occlusion rule.

**Alternatives considered:**
- **Tree-family or vegetation-family heuristics:** rejected because the user asked for the effect behind any front object.
- **Bounding-box occluders unrelated to render order:** rejected because it can disagree with what the scene actually draws.

### Use an occluder mask plus a tinted player overlay

The shell should render the player silhouette through a mask built from front occluders. The silhouette pass should only show pixels where both of these are true: the player sprite occupies the pixel, and a front occluder also covers it.

**Why:** This directly matches the requested visual effect: only the hidden portion of the player becomes visible as a faded overlay.

**Alternatives considered:**
- **Fade the whole player when any object is in front:** rejected because the visible portion should remain normal.
- **Fade the whole front object instead:** rejected because it changes the occluder rather than revealing the player.

### Prefer offscreen compositing with existing SFML capabilities

Implementation should use offscreen render textures and/or a small shader-based composite built from existing SFML rendering primitives.

**Why:** The effect needs pixel-accurate overlap between the player sprite and front occluders. Offscreen compositing expresses that requirement cleanly without changing asset data.

**Alternatives considered:**
- **Pure CPU-side geometry clipping:** rejected because atlas-driven multi-part vegetation makes pixel-accurate clipping awkward and brittle.

## Risks / Trade-offs

- **[Occlusion compositing may add render-path complexity]** → Mitigation: keep the effect isolated to a dedicated silhouette pass after normal scene drawing.
- **[Pixel-accurate masks may have edge artifacts]** → Mitigation: use the exact same sprite transforms and atlas cells for the occluder mask as the normal world render path.
- **[The effect could accidentally break y-sort semantics]** → Mitigation: derive front occluders from the existing y-sorted queue instead of creating a second ordering model.

## Migration Plan

1. Update the generated-content rendering requirement to preserve y-sort while adding partial player silhouettes through front occluders.
2. Implement the silhouette pass in the SFML shell using the existing y-sorted queue as the occluder source.
3. Add focused tests around queue-derived occluders and unchanged base ordering.
4. If rollback is needed, remove the silhouette composite pass and keep the baseline y-sorted scene render path.

## Open Questions

- Whether the final silhouette should be implemented with a dedicated fragment shader, blend-mode composition, or a hybrid of both should be resolved during implementation based on the cleanest repo-native SFML path.
