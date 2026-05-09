## Context

The runtime already keeps SFML-facing concerns in `Game` and gameplay-facing coordination in `OverworldRuntime`. `Game` owns event polling, frame pacing, and drawing, while `OverworldRuntime` assembles repo-native render snapshots from `World`, `Player`, and `Camera`. The proposed debug overlay crosses those boundaries: the shell should own overlay input and SFML text rendering, but the values displayed by the overlay should still come from gameplay-owned runtime data instead of ad hoc shell-side inspection.

The overlay also introduces a new runtime asset dependency. The repository already bundles two fonts under `assets/output/fonts/`, but the build currently stages only terrain, vegetation, player sprite, and classification assets into the executable-local `assets/` directory. The overlay needs one of those bundled fonts available at runtime.

## Goals / Non-Goals

**Goals:**
- Add a developer-facing debug overlay that can be toggled with `F1`.
- Keep movement input, zoom controls, and grid-toggle controls working while the overlay is visible.
- Render the overlay as a semi-transparent black panel with readable text for frame rate, loaded generated-content count, rendered generated-content count, player tile coordinates, and zoom level.
- Preserve the existing boundary where `Game` handles SFML concerns and `OverworldRuntime` publishes repo-native runtime data.
- Stage a bundled font asset so the overlay can render from the built executable's `assets/` directory.

**Non-Goals:**
- Adding a pause menu, modal debug UI, or interactive widgets.
- Generalizing the overlay into a full diagnostics or console system.
- Changing gameplay movement, chunk generation rules, or tile-grid behavior beyond ensuring those controls still operate while the overlay is visible.
- Introducing a new asset loading framework or external text-rendering dependency.

## Decisions

### Overlay visibility stays in the SFML shell
`Game` will own a dedicated overlay visibility flag toggled by `F1`. This flag will remain separate from gameplay-owned `debugViewState`, because the overlay is a shell-rendered HUD rather than a gameplay rule. This avoids coupling the HUD to the zoom/grid debug state and keeps the overlay from changing gameplay behavior.

**Alternatives considered**
- Reusing `debugViewState.isEnabled`: rejected because that state already affects gameplay-facing zoom behavior and would make overlay visibility implicitly drive unrelated debug rules.
- Moving overlay visibility into `OverworldRuntime`: rejected because the overlay itself is a shell-rendered presentation concern.

### Gameplay-derived overlay values publish through a dedicated debug snapshot
`OverworldRuntime` will publish a small repo-native debug snapshot alongside the existing render snapshot. That snapshot will carry player tile coordinates, zoom percentage, loaded generated-content count, and rendered generated-content count for the active frame. `Game` will continue to consume `OverworldRuntime` instead of reading `World`, `Player`, or `Camera` directly.

The loaded-object counter will count retained generated-content instances across currently loaded world chunks. The rendered-object counter will count visible generated-content entries included in the active frame snapshot.

**Alternatives considered**
- Having `Game` inspect `World`, `Player`, and `Camera` directly: rejected because it breaks the existing gameplay boundary and duplicates knowledge already owned by `OverworldRuntime`.
- Embedding debug values into the render snapshot itself: possible, but rejected in favor of a separate debug snapshot because the overlay data is diagnostic metadata rather than render geometry.

### Overlay rendering stays as a passive final draw pass
The shell will render the overlay after the overworld scene using the default UI view. The panel will use a black rectangle with partial alpha and a small text stack rendered on top. The overlay will never capture input or short-circuit the frame lifecycle; event processing and update sequencing remain unchanged whether the overlay is visible or not.

**Alternatives considered**
- Rendering the overlay inside the world-space view: rejected because the HUD should stay screen-anchored instead of moving with the camera.
- Treating the overlay as a modal layer: rejected because the requirement is that controls continue working while it is visible.

### Use a bundled pixel font staged with runtime assets
The overlay will load one of the bundled fonts from `assets/output/fonts/` and rely on the existing runtime asset staging pattern so the executable can resolve the font from its local `assets/` directory. A single regular-weight font is sufficient for the initial overlay.

**Alternatives considered**
- Using a system-installed font: rejected because it would make runtime behavior platform-dependent.
- Loading both bundled fonts immediately: rejected because the initial overlay only needs one font and should keep asset staging minimal.

### Frame-rate display should be readable rather than noisy
The overlay will display a shell-computed frame-rate value derived from recent frame timing rather than from gameplay state. The exact smoothing strategy can stay simple, but the rendered value should be stable enough to read during normal play instead of changing erratically every frame.

**Alternatives considered**
- Showing only instantaneous frame time: rejected because it is harder to scan during active movement.
- Publishing FPS through gameplay: rejected because frame pacing is owned by the shell, not gameplay modules.

## Risks / Trade-offs

- **[Runtime boundary drift]** Exposing overlay data by reaching into gameplay modules from `Game` would weaken the shell/gameplay split. → **Mitigation:** publish a dedicated debug snapshot from `OverworldRuntime` and keep `Game` consuming that boundary.
- **[Counter ambiguity]** “Objects loaded” could be interpreted as tiles, markers, or generated content. → **Mitigation:** define both counters explicitly around generated-content instances only and reflect that wording in specs and UI text.
- **[Asset staging regressions]** Adding a font dependency can fail at runtime if the build does not copy the font into the executable-local asset tree. → **Mitigation:** extend runtime asset staging checks and document the staged font requirement in the asset-layout spec delta.
- **[Unreadable overlay]** Text may be hard to read over bright terrain if transparency is too high. → **Mitigation:** require a semi-transparent black panel with opaque high-contrast text.

## Migration Plan

1. Add spec deltas for input translation, runtime asset staging, render/debug snapshots, and the new overlay capability.
2. Extend runtime asset staging to copy the chosen overlay font into the built executable's `assets/` directory.
3. Add the debug snapshot boundary in `OverworldRuntime` and expose the loaded/rendered object counters plus player tile and zoom values.
4. Add shell-side overlay state, FPS tracking, font loading, and final-pass overlay drawing in `Game`.
5. Add or update focused tests covering input translation, asset staging expectations, and any snapshot helpers introduced for overlay data.

Rollback is straightforward: remove the overlay render pass, stop staging the font asset, and revert the added debug snapshot fields without changing save data or external APIs.

## Open Questions

- The frame-rate display should be readable, but the precise smoothing window can be finalized during implementation as long as it remains stable and inexpensive.
