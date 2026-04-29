## Context

The current executable creates an `sf::RenderWindow` directly in `main.cpp`, polls events inline, and clears/displays the window in a single function. That structure is sufficient for a blank window, but it does not provide stable ownership boundaries for the game loop or clear integration points for future systems such as the world model, player control, camera management, and procedural generation.

The design needs to stay small enough for the current codebase while establishing a durable runtime shape. The project uses C++20 and SFML 3, and the current scope is a single executable with no editor, scripting runtime, or plugin system. This change also needs to define the basic C++ structure conventions for the runtime layer, including file placement, namespace usage, and public-header dependency boundaries.

## Goals / Non-Goals

**Goals:**
- Introduce a top-level runtime object responsible for the window lifecycle and main loop.
- Separate loop phases into event processing, update, and render so later gameplay systems can plug into predictable lifecycle points.
- Define initial ownership boundaries between the runtime coordinator and domain objects such as world, player, and camera.
- Keep the entry point minimal so application startup and gameplay runtime are no longer coupled.

**Non-Goals:**
- Implement procedural generation, chunk streaming, combat, UI, or save/load systems.
- Introduce an ECS, scene graph, scripting layer, or multi-threaded engine architecture.
- Finalize all downstream gameplay object APIs beyond what is needed to establish the runtime seam.

## Decisions

### Introduce a `Game` class as the runtime entry object

The runtime will move from `main.cpp` into a dedicated `Game` class that owns the SFML window and exposes a `run()` method. `main.cpp` should be responsible only for constructing `Game`, invoking the runtime, and returning an exit status.

This keeps startup simple while removing orchestration concerns from the entry point.

**Alternatives considered**
- Keep the loop in `main.cpp` and extract helper functions: rejected because ownership would still be implicit and future runtime responsibilities would remain scattered.
- Add a broader engine/application layer now: rejected because it adds abstraction before the project has enough systems to justify it.

### Use a split public/private file layout for the runtime

The first runtime class should be declared in `include/main/Game.hpp` and implemented in `src/main/Game.cpp`. Public declarations belong in `include/`, while executable-specific implementation files stay in `src/`.

This gives the project a predictable structure early and makes later growth into additional runtime headers and source files straightforward.

**Alternatives considered**
- Keep `Game.hpp` beside `Game.cpp` under `src/`: rejected because it blurs the public/private boundary and does not establish the intended header layout.

### Use project-scoped namespaces for runtime types

Runtime types should be declared under a project namespace instead of the global namespace. For this change, `Game` and related runtime types should live under a project-scoped namespace such as `rpg`, with deeper nesting added only when it meaningfully improves clarity.

This avoids global-symbol pollution without coupling namespace shape too tightly to the current folder layout.

**Alternatives considered**
- Leave runtime classes in the global namespace: rejected because it does not scale as more gameplay types are introduced.
- Mirror the directory layout exactly in namespaces: deferred because the project does not yet have enough modules to justify deep namespace nesting.

### Use explicit loop phases: process events, update, render

The runtime loop should call separate methods for event handling, simulation updates, and rendering. The update phase should accept frame delta time so movement and simulation can become frame-rate independent as gameplay logic is introduced.

This creates clear extension points and avoids mixing input polling, gameplay state mutation, and drawing in one block.

**Alternatives considered**
- Single loop body with inline logic: rejected because it scales poorly as more systems are added.
- Fixed-timestep simulation from the start: deferred because the current project does not yet need the extra complexity, though the design should leave room for it later.

### `Game` coordinates systems but does not own their internal rules

`Game` will orchestrate lifecycle flow and top-level ownership, but world rules, player behavior, and camera behavior should live in their own types. `Game` may call `world.update(...)` or `player.update(...)`, but it should not become the place where collision logic, biome generation, or rendering policy are implemented.

This keeps the top-level runtime from turning into a monolith.

**Alternatives considered**
- Put early gameplay logic directly in `Game`: rejected because it makes later extraction harder and encourages a god object.

### Start with direct object composition, not a generalized engine pattern

The first architecture should compose a small number of concrete types directly from `Game` rather than introducing abstract subsystems, service locators, or ECS-style registration. World, player, and camera can start as normal objects or placeholders owned by the runtime.

This matches the project's current size and keeps implementation straightforward.

**Alternatives considered**
- Build a subsystem registry or service container: rejected because it solves problems the codebase does not have yet.
- Adopt ECS immediately: rejected because it would dominate the architecture discussion before core gameplay needs are known.

### Keep public headers lightweight and avoid SFML includes when possible

Public runtime headers should avoid including SFML headers when declarations can be expressed through forward declarations and implementation indirection. Because `Game` owns the window lifecycle, this likely means storing SFML-owned state behind an incomplete type boundary rather than embedding concrete SFML types directly in the public header.

This reduces compile-time coupling and keeps the runtime boundary focused on the project's own types.

**Alternatives considered**
- Include SFML types directly in `Game.hpp`: rejected because it leaks framework dependencies through the public boundary and makes later refactors harder.
- Hide all runtime state behind a broader application abstraction: rejected because it adds more indirection than the project currently needs.

### Use file banners and classic include guards for the initial runtime headers

New runtime source and header files should use the project's file banner/license block, and headers should use explicit include guards such as `RPG_MAIN_GAME_HPP`.

This creates a consistent convention for the codebase as it expands beyond a single source file.

**Alternatives considered**
- Omit file banners and rely only on repository-level licensing: rejected because the requested project convention is to carry the banner into source files.
- Use `#pragma once`: deferred in favor of explicit guards because that is the requested convention for this codebase.

### Document the public runtime interface with Doxygen

The public `Game` header should include Doxygen comments for the namespace, class, lifecycle methods, and the implementation boundary so the header serves as the primary API contract for the runtime.

This keeps the public surface understandable as more gameplay systems are introduced behind the runtime coordinator.

**Alternatives considered**
- Leave public declarations undocumented: rejected because the runtime header is intended to be the stable entry point for the executable.

## Risks / Trade-offs

- **[`Game` grows into a god object]** → Keep `Game` focused on lifecycle and ownership only; move behavior into domain classes as soon as it appears.
- **[Early APIs for world/player/camera are premature]** → Define only minimal interfaces needed by the runtime and evolve them as gameplay features arrive.
- **[Variable delta time can complicate future simulation]** → Use delta time now for simplicity, but keep loop structure flexible so a fixed-step update can be introduced later if needed.
- **[Forward-declaration boundaries may require indirection]** → Accept a small amount of pointer or private-state complexity to keep SFML dependencies out of public headers.
- **[Architecture work delays visible gameplay]** → Keep the initial runtime scope narrow and use it immediately as the base for the first playable overworld slice.
