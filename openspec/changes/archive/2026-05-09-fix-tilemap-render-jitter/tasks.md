## 1. Frame pacing and runtime shell

- [x] 1.1 Keep the existing event -> update -> render frame lifecycle in `Game` so player movement and walk animation cadence remain responsive.
- [x] 1.2 Make the window pacing configuration explicit so the runtime uses one active frame pacing strategy at a time.
- [x] 1.3 Make the window pacing configuration explicit so the runtime uses either vertical sync or a framerate limiter, but not both at the same time.

## 2. Pixel-stable presentation

- [x] 2.1 Update camera/render snapshot publication so the render-facing overworld camera frame can be aligned to the render pixel grid without changing gameplay tracking behavior.
- [x] 2.2 Apply zoom-aware pixel-aligned camera presentation so terrain remains visually stable while the player moves and when zoom is changed.
- [x] 2.3 Add or update focused tests covering explicit pacing configuration and zoom-aware pixel-stable camera presentation requirements.

## 3. Render snapshot coherence and tests

- [x] 3.1 Use one published render-facing camera frame consistently for visibility collection and drawing.
- [x] 3.2 Keep the retained change scoped to zoom-aware camera snapping instead of reintroducing fixed-step or interpolation machinery.
- [x] 3.3 Validate the resulting overworld behavior against the stable-overworld-rendering scenarios and adjust task scope if the retained snapping approach resolves the visible shimmer.
