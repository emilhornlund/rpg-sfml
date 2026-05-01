## Context

The current runtime split already gives the project strong gameplay-facing seams: `World` owns deterministic terrain and chunk retention, `Player` owns movement state, and `Camera` owns framing. The remaining architectural pressure sits in `Game.cpp`, which still initializes the overworld session, polls live movement intent from SFML, sequences player and camera updates, and assembles render-facing state directly from gameplay objects.

That shape is workable for the current slice, but it places the next layer of procedural-world work on the wrong side of the boundary. If spawn rules, session flow, map interactions, NPC updates, or render-facing entity state continue to grow in `Game.cpp`, the SFML runtime shell becomes the accidental gameplay coordinator. This change creates a dedicated overworld runtime boundary before those concerns expand further.

## Goals / Non-Goals

**Goals:**
- Introduce a dedicated overworld coordinator that owns the active `World`, `Player`, and `Camera` collaboration for one gameplay session.
- Reduce `Game` to an executable shell that handles SFML window lifecycle, input/event collection, and drawing from gameplay-provided state.
- Establish stable repo-native types for overworld input and render-facing frame state so gameplay rules can evolve without direct SFML coupling.
- Preserve the current overworld behavior and existing module boundaries while making future procedural-world features easier to place.

**Non-Goals:**
- Replacing the current `World`, `Player`, or `Camera` modules with a larger engine-style framework.
- Adding persistence, combat, NPC simulation, inventory, or quest systems in this change.
- Introducing asynchronous simulation, background chunk generation, or multithreaded rendering.
- Reworking the public `Game` API beyond what is necessary to keep it a thin runtime shell.

## Decisions

### Decision: Introduce an overworld-specific runtime collaborator

The runtime should add a dedicated collaborator such as `OverworldRuntime` or `OverworldSession` that owns the active `World`, `Player`, and `Camera` instances and exposes the small set of operations the SFML shell needs: initialize session state, advance one frame from input, and expose render-facing frame data.

**Why:** This pulls gameplay orchestration behind one boundary without disturbing the existing module boundaries that already separate world, player, and camera responsibilities.

**Alternatives considered:**
- **Keep orchestration in `Game.cpp` and only extract helpers:** rejected because it would keep session rules fragmented across the SFML shell.
- **Push orchestration into `World`:** rejected because world ownership would absorb player, camera, and frame lifecycle concerns that do not belong there.
- **Introduce a broad engine/application layer:** rejected because it is too large for the current repo and would overfit the project at this stage.

### Decision: Keep SFML types at the outer shell

The new overworld coordinator should consume repo-native input/state types and produce repo-native render-facing data. `Game.cpp` remains responsible for polling SFML events/keyboard state and translating them into those types, plus drawing the resulting frame through SFML objects.

**Why:** The project already protects `Game.hpp` from SFML-heavy dependencies. Extending that boundary inward keeps gameplay rules testable and prevents SFML details from leaking into the procedural-world architecture.

**Alternatives considered:**
- **Let the overworld coordinator read SFML keyboard state directly:** rejected because it couples gameplay simulation to a concrete window/input library.
- **Move SFML rendering primitives into gameplay modules:** rejected because it mixes gameplay-facing state with framework rendering details.

### Decision: Model frame rendering through a snapshot-style boundary

The overworld coordinator should provide a render-facing snapshot for the current frame, including camera framing, visible terrain tiles, and player marker placement. `Game` should render from that snapshot instead of deriving those values itself from `World`, `Player`, and `Camera`.

**Why:** This makes the gameplay/runtime boundary explicit and gives later features such as NPC markers, props, or interaction highlights a natural place to appear without expanding the SFML shell.

**Alternatives considered:**
- **Let `Game` keep pulling from each gameplay module directly:** rejected because it preserves the current orchestration leak.
- **Return SFML draw commands or shapes from gameplay code:** rejected because it couples gameplay output to the rendering backend.

### Decision: Keep behavior-preserving migration as the first pass

The first implementation should preserve the current spawn flow, player movement rules, camera follow behavior, and visible terrain rendering while changing only where those decisions live.

**Why:** This change is architectural. Keeping outward behavior stable makes it easier to verify that the new boundary is sound before layering new procedural-world features on top.

**Alternatives considered:**
- **Combine the coordinator extraction with new gameplay features:** rejected because it would blur architectural regressions with new-feature bugs.
- **Wait until more features exist before extracting the coordinator:** rejected because the cost and coupling increase with every new gameplay concern added to `Game.cpp`.

## Risks / Trade-offs

- **New coordinator becomes a second god object** → Mitigation: keep its public surface narrow and leave world generation, movement rules, and camera logic in their existing modules.
- **Render snapshots may duplicate transient data each frame** → Mitigation: keep the first snapshot minimal and limited to data `Game` already derives today.
- **Input translation boundary may initially feel thin** → Mitigation: start with only the input needed for the current slice and expand it incrementally as more gameplay verbs appear.
- **Tests may need to move from file-boundary checks to behavior-boundary checks** → Mitigation: preserve the existing boundary checks and add focused tests around the new overworld coordinator responsibilities.

## Migration Plan

1. Add the new overworld coordinator type and the repo-native input/render snapshot types it depends on.
2. Move overworld initialization, player/camera update sequencing, and render-facing state assembly out of `Game.cpp` into that coordinator.
3. Keep `Game` responsible for SFML event processing, movement-intent collection, and drawing from the coordinator's current frame state.
4. Update tests and boundary checks to assert that `Game` remains a shell and the overworld flow is owned by the dedicated coordinator.
5. Use the extracted boundary as the landing point for later procedural-world features such as entity markers, interactions, and chunk-aware session behaviors.

## Open Questions

- Should the new type be named `OverworldRuntime`, `OverworldSession`, or something else that better signals “active gameplay slice” rather than “entire game”?
- Should player marker placement remain a rendering helper in shared runtime support, or move into the overworld render snapshot boundary with the rest of the frame data?
- When additional gameplay systems arrive, should they hang directly off the overworld coordinator or through a smaller world-session state object owned by it?
