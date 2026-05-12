## 1. Overlap-qualified occlusion candidates

- [x] 1.1 Add render-side helpers that derive player presentation bounds and filter broad front occluders down to overlap-qualified occlusion candidates from snapshot-published world-space geometry.
- [x] 1.2 Switch the occlusion trigger in `Game::render()` from broad front-occluder existence to overlap-qualified candidate existence while preserving current silhouette output when overlap is present.
- [x] 1.3 Publish both broad front-occluder counts and overlap-qualified occlusion candidate counts through the render/debug diagnostics path.

## 2. Reduced-resolution occlusion composition

- [x] 2.1 Introduce reduced-resolution player and occluder mask sizing derived from the active window dimensions for the occlusion pass.
- [x] 2.2 Update the occlusion composite path so reduced-resolution masks remain aligned with the player's on-screen presentation across resize and zoom changes.
- [x] 2.3 Keep the current silhouette-only behavior by validating that unobstructed player pixels remain unchanged while overlap-qualified occluded pixels still render the silhouette overlay.

## 3. Validation and follow-up guardrails

- [x] 3.1 Update render-path and debug-overlay tests to cover overlap-qualified occlusion candidates, the new overlay metric, and reduced-resolution resize-safe silhouette composition.
- [x] 3.2 Run the documented build and test commands and verify the occlusion optimization path remains correct under fullscreen and zoom stress scenarios.
- [x] 3.3 Document any remaining performance gap and whether the next follow-up should prioritize player-local masks or cheaper occluder-mask proxy geometry.
