## Context

The current runtime bootstraps successfully, but `src/main/Game.cpp` still declares `World`, `Player`, and `Camera` as empty local structs near the loop coordinator. That keeps the project moving for now, but it also creates the wrong growth path: as gameplay logic appears, the easiest place to add it will be inside `Game.cpp`.

This change introduces dedicated gameplay module files before those placeholders gain real behavior. The design needs to preserve the existing `Game` orchestration role, keep the public `Game` header lightweight, and avoid inventing deeper gameplay systems than the project currently needs.

## Goals / Non-Goals

**Goals:**
- Move `World`, `Player`, and `Camera` into dedicated runtime-facing types with clear file ownership.
- Keep `Game` responsible for startup, frame sequencing, and collaborator coordination rather than gameplay internals.
- Define a module layout that can absorb future world, player, and camera logic without forcing another structural rewrite.

**Non-Goals:**
- Implement procedural generation, movement, camera behavior, or rendering rules beyond the current placeholders.
- Introduce an ECS, scene graph, or broader engine framework.
- Change the executable entry point or public `Game` API unless required by the new internal boundaries.

## Decisions

### Extract gameplay placeholders into dedicated domain files
`World`, `Player`, and `Camera` will become real project types in their own headers and source files rather than anonymous local structs inside `Game.cpp`.

**Why:** This creates an explicit place for each gameplay concern to grow and makes the architecture visible in the file tree.

**Alternative considered:** Keep the placeholder structs in `Game.cpp` until they gain behavior. Rejected because it encourages future gameplay logic to accumulate in the coordinator and makes later extraction noisier.

### Keep `Game` as the frame coordinator
`Game` will continue to own the main loop, window lifecycle, and frame sequencing. It should construct and retain the gameplay modules, invoke them at the appropriate phases, and avoid owning their internal rules.

**Why:** The project already centers the executable runtime around `Game`, and preserving that boundary keeps the public runtime story simple.

**Alternative considered:** Move more logic into `Game` for now and refactor later. Rejected because the requested change exists specifically to prevent `Game` from becoming a catch-all runtime object.

### Prefer simple composition over a new abstraction layer
The extracted module types should be wired together through direct ownership or straightforward references, without adding registries, service locators, or generalized orchestration frameworks.

**Why:** The codebase is still early-stage, so indirection would add complexity without solving a current problem.

**Alternative considered:** Introduce a subsystem manager or ECS-style registration point. Rejected as premature for three module types with placeholder behavior.

## Risks / Trade-offs

- **More files for little runtime behavior today** -> Accept the short-term overhead to establish the long-term architecture before gameplay logic lands.
- **Module interfaces may need to change soon as real systems emerge** -> Keep the initial interfaces narrow and coordinator-focused so refactors stay localized.
- **Boundary confusion between runtime and gameplay responsibilities** -> Document the expectation in specs that `Game` coordinates phases while module internals own their own rules.

## Migration Plan

1. Add dedicated files for `World`, `Player`, and `Camera`.
2. Update runtime build wiring so those files are compiled with the executable.
3. Replace inline placeholder declarations in `Game.cpp` with the extracted types.
4. Keep runtime behavior unchanged while confirming the coordinator/module boundary is in place.

No rollout or rollback process is needed beyond reverting the internal refactor if necessary.

## Open Questions

- Should the first extracted module APIs stay as pure data holders, or should they immediately expose no-op update/render hooks to formalize phase ownership?
- Should camera ownership remain directly under `Game`, or should it eventually derive some state from `Player` and `World` once movement is added?
