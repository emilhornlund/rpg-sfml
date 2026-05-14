## Context

`Game.cpp` is already thinner than the original vertical-slice runtime, but it still centralizes most executable-shell responsibilities in one translation unit. The file owns SFML resources, translates window/input events, accumulates frame diagnostics, assembles the render plan, composites player occlusion, and draws the debug overlay.

The repository already has helper-oriented boundaries such as `GameRuntimeSupport`, `GameRenderSupport`, `GameRenderBatchSupport`, and `PlayerOcclusionSilhouetteSupport`. The next step is not a large architecture rewrite; it is a controlled decomposition that keeps `Game` as the top-level SFML shell while moving cohesive shell-owned concerns into support modules that can be tested independently and reviewed in smaller slices.

Two constraints shape the design:

- `include/main/Game.hpp` must remain the stable public entry point and continue hiding SFML-heavy details behind `Game::Impl`.
- Existing boundary tests currently assert for literal `Game.cpp` patterns, especially around rendering and occlusion, so test expectations must evolve before deeper extractions can land safely.

## Goals / Non-Goals

**Goals:**
- Reduce the amount of concern-mixing in `Game.cpp` without changing the public `Game` API or overworld behavior.
- Sequence the decomposition so the first extraction is low risk and later extractions are not blocked by test shape.
- Preserve the current module boundaries where `Game` remains the shell and `OverworldRuntime` remains the gameplay-facing collaborator.
- Increase the amount of runtime-shell logic that can be tested through focused support-module tests instead of `Game.cpp` literal checks.

**Non-Goals:**
- Replacing the existing `Game` / `OverworldRuntime` boundary with a new runtime architecture.
- Moving gameplay ownership back into `Game.cpp` or introducing new gameplay-facing module layers.
- Changing visible debug overlay behavior, player-occlusion behavior, render ordering, or asset layout as part of the decomposition itself.
- Switching the build system, vendored SFML flow, or public executable structure.

## Decisions

### 1. Extract the debug overlay pipeline first

The first implementation slice will target the debug overlay because it is the most isolated remaining concern in `Game.cpp`: it is screen-space only, already relies on helperized formatting/state logic, and can be extracted without rewriting render-plan or occlusion flow first.

**Why this over occlusion-first**
- The overlay draw block is localized and low-coupling.
- The supporting logic already exists in `GameRuntimeSupport`.
- The occlusion pass is entangled with render-plan assembly, sprite reuse, reduced-resolution surfaces, and boundary tests, so it is better treated as a later slice.

### 2. Keep `Game` as an orchestration shell, not a pass-through wrapper

After decomposition, `Game` will still coordinate frame phases and own `Game::Impl`, but the implementation should mostly make high-level calls into shell-support collaborators rather than inlining every detailed step.

**Why this over fully externalizing the runtime shell**
- It preserves the repository's current `Game` boundary.
- It avoids speculative abstraction layers.
- It matches the repo's existing pattern of small support helpers around a stable top-level coordinator.

### 3. Group extraction targets by shell concern, not by “render everything” versus “runtime everything”

The decomposition will be organized around cohesive shell-owned concerns:
- overlay pipeline
- resource bootstrap
- event translation
- render-plan assembly
- occlusion compositing
- world-pass rendering

**Why this over a single monolithic renderer extraction**
- Smaller tickets are easier to review and safer to validate.
- Each concern already has partial helper precedent in the codebase.
- It avoids replacing one large `Game.cpp` blob with one large “renderer” blob.

### 4. Unpin boundary tests before moving render-plan and occlusion code

Before extracting the deeper render concerns, tests that assert literal `Game.cpp` content should shift toward module-boundary and behavior assertions that tolerate relocation of helper-owned logic.

**Why this before render-plan/occlusion extraction**
- Current tests explicitly require certain code paths to remain inline in `Game.cpp`.
- Refactoring without updating those checks would create churn or force unnatural code placement.

### 5. Preserve existing support modules where they fit, and add new helpers only where a concern lacks a natural home

The decomposition should extend or complement the current support modules instead of introducing a new runtime framework. New support files are acceptable when an extracted concern does not fit the current helpers cleanly.

**Alternatives considered**
- **Fold everything into `GameRuntimeSupport`**: rejected because that file already spans input, frame utilities, overlay text, render ordering, and misc runtime helpers.
- **Introduce a single `GameRenderer` class**: rejected because it would likely re-centralize the same complexity in a different file.
- **Extract occlusion first**: rejected as a higher-risk first move due to its coupling with render planning and test expectations.

## Risks / Trade-offs

- **Boundary-test churn** → Update tests intentionally before moving deeper render logic so refactors are blocked by meaningful boundaries instead of file-literal placement.
- **Helper sprawl** → Keep concern ownership explicit and avoid adding vague “misc support” files.
- **Partial extraction leaving `Game.cpp` awkwardly split** → Sequence the work so each ticket leaves a cleaner coordinator shape, not an intermediate tangle.
- **Accidental behavior changes during refactor** → Preserve existing helper logic, reuse current state structures, and keep extractions behavior-neutral unless a later change explicitly alters requirements.

## Migration Plan

1. Extract the debug overlay pipeline and keep runtime behavior unchanged.
2. Extract resource bootstrap and event translation so constructor/setup and event polling shrink independently of render work.
3. Update boundary tests to validate module ownership without pinning render-plan and occlusion code to `Game.cpp`.
4. Extract render-plan assembly, then player-occlusion compositing, then world-pass rendering.
5. Leave `Game.cpp` as the orchestrator that wires together the extracted shell concerns.

Rollback is straightforward at each step because each ticket is intended to be a small internal refactor with no public API changes.

## Open Questions

- Should the extracted overlay pipeline own FPS accumulation state entirely, or should `Game` continue to own frame-rate counters and only delegate drawing/layout?
- Does render-plan assembly fit better as an extension of `GameRuntimeSupport`, or should it become a dedicated support module once extracted?
- When the boundary tests are updated, should they move toward focused helper tests only, or should one lightweight `Game.cpp` shell-boundary check remain to prevent regressions in coordinator responsibilities?
