## Context

The current runtime already separates most gameplay concerns from the SFML shell. `OverworldRuntime` accepts `OverworldInput`, `Player` consumes `MovementIntent`, and render-facing data flows back through repo-native types. The remaining leak is in `Game.cpp`, where SFML keyboard polling is used directly to construct movement intent for gameplay updates.

This change needs to tighten that last seam without expanding scope into input rebinding, action systems, or broader gameplay command modeling. The repository already has a pattern for backend translation at the edge through `RuntimeEvent`, so input translation should follow the same boundary style.

## Goals / Non-Goals

**Goals:**
- Keep SFML key polling at the executable shell boundary.
- Pass repo-native overworld input into gameplay coordination.
- Make input translation easier to test without coupling gameplay code to SFML polling details.
- Preserve the existing runtime/module layout and avoid unnecessary architectural expansion.

**Non-Goals:**
- Introduce configurable key bindings or device abstraction.
- Redesign player movement rules or replace `MovementIntent`.
- Introduce a broader action-command system unless current requirements need it.
- Move rendering or window lifecycle responsibilities out of `Game`.

## Decisions

### Use `OverworldInput` as the boundary type

The runtime already exposes `OverworldInput` as the per-frame gameplay input contract. Reusing that type keeps the design aligned with the current module boundaries and avoids introducing a parallel command model before the gameplay slice needs one.

**Alternatives considered**
- **Introduce `PlayerCommand` now:** More semantic for future one-shot actions, but adds abstraction before the current slice needs more than movement plus viewport data.
- **Pass SFML key state inward:** Rejected because it would preserve backend coupling in gameplay-facing code and reduce portability.

### Translate SFML input in a dedicated edge adapter

The SFML shell should gather backend-specific key state, then immediately translate it into repo-native overworld input before calling the overworld runtime. The translation logic may live in a small runtime support helper or another repo-native runtime file, but it should remain conceptually owned by the shell boundary rather than the gameplay modules.

**Alternatives considered**
- **Inline translation directly in the update method:** Better than leaking SFML deeper, but still keeps backend mapping logic entangled with frame orchestration.
- **Push translation into `OverworldRuntime` or `Player`:** Rejected because those modules are gameplay-facing boundaries and should not know about SFML-specific polling.

### Keep input modeled as a per-frame snapshot

For the current overworld slice, movement is continuous and already represented as `MovementIntent`. The input contract should remain a per-frame snapshot that describes the current movement state and viewport data. If one-shot actions are added later, the boundary can grow deliberately rather than prematurely adopting a generic command system.

**Alternatives considered**
- **Adopt event-style commands for all input now:** Useful for richer action sets, but not justified for the current movement-only slice.

## Risks / Trade-offs

- **Future actions may outgrow the current shape** → Keep the translation layer narrow so the boundary can evolve from `OverworldInput` toward richer commands later without re-entangling SFML with gameplay code.
- **A helper placed poorly could become a dumping ground** → Keep the adapter focused on translation only and leave gameplay interpretation in runtime or module code.
- **Tests could accidentally validate implementation detail instead of contract** → Center tests on repo-native input results and gameplay-facing consumption rather than on SFML internals.
