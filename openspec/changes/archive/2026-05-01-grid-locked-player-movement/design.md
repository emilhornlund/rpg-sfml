## Context

The current overworld movement path is built around a free-form `MovementIntent` vector. The SFML shell polls directional key state, `GameRuntimeSupport` converts that state to an `(x, y)` intent, and `Player` normalizes the vector to move through world space by pixel distance. That works for smooth movement, but it creates two behaviors that now conflict with the intended game feel:

- perpendicular key combinations produce diagonal travel even though the player only has four directional animations
- the player can stop or drift at arbitrary pixel positions instead of following tile centers

The codebase already exposes the primitives needed for grid-locked movement. `World` can convert between tile coordinates and tile centers, player spawn already begins on a tile center, and the overworld runtime plus camera already consume the player's logical world position without needing free-form movement semantics.

The main constraint is input priority. "Last pressed wins" cannot be derived from a stateless snapshot of held keys alone, so the shell boundary must preserve enough ordering information to resolve simultaneous held directions deterministically while still keeping SFML event types out of gameplay-facing code.

## Goals / Non-Goals

**Goals:**

- enforce single-direction overworld movement with no diagonal travel
- keep the player traveling only along segments between adjacent tile centers
- finish an in-progress tile step even if movement input is released before the destination center is reached
- preserve overworld runtime and rendering boundaries by keeping SFML details at the shell edge
- make the new movement rules directly testable through gameplay and input translation tests

**Non-Goals:**

- adding eight-direction movement, diagonal animation rows, or analog speed control
- redesigning camera behavior, sprite assets, or world generation rules
- introducing turn buffering beyond selecting the next step from currently held input at tile centers
- changing how traversable versus blocked terrain is generated

## Decisions

### Use a tile-step movement model inside `Player`

`Player` will stop treating movement intent as a normalized travel vector. Instead, it will manage movement as a discrete step from a starting tile center to a destination tile center.

Rationale:
- this makes grid alignment an invariant instead of a best-effort result
- traversability becomes a single destination-tile decision rather than repeated pixel-space probing
- auto-completing the current step on input release becomes natural: once a step starts, the destination is fixed until reached

Alternatives considered:
- **Clamp free movement back to tile centers after release**: simpler on paper, but still allows diagonal drift and makes correction behavior harder to reason about
- **Keep pixel stepping with axis constraints**: preserves existing code shape, but fights the intended tile-by-tile feel and collision rules

### Resolve direction choice at the shell boundary using press-order state

The shell boundary will continue owning backend-specific key and event handling, but it will also retain enough directional ordering state to publish one active cardinal direction when multiple keys are held. The chosen rule is last-pressed-wins.

Rationale:
- last-pressed-wins was the selected control feel
- this policy cannot be derived reliably from a pure boolean snapshot of held keys
- keeping the ordering state in the shell preserves the existing boundary where gameplay consumes repo-native input instead of SFML event details

Alternatives considered:
- **Choose a fixed axis priority**: simpler, but explicitly not the selected behavior
- **Resolve priority inside gameplay from booleans only**: impossible to implement faithfully without additional ordering information

### Only change direction when a tile step completes

The active input direction will be sampled when the player is idle at a tile center or when an in-progress step completes. A new perpendicular key pressed mid-step will affect the next step, not the current one.

Rationale:
- preserves the classic tile-walk feel
- avoids snapping or curving away from the current segment
- keeps movement state easy to test because the player is either idle at a center or committed to one destination

Alternatives considered:
- **Allow mid-step retargeting**: feels less grid-locked and complicates auto-finish guarantees

### Block movement by refusing to start invalid steps

If the destination tile in the chosen direction is not traversable, the player will remain at the current tile center and no movement step will begin.

Rationale:
- aligns collision checks with tile-based motion
- avoids the current partial sliding behavior, which does not match the requested path restriction

Alternatives considered:
- **Retain axis sliding into a valid component**: useful for free movement, but incorrect for strict tile stepping

## Risks / Trade-offs

- **[Input ordering state becomes more complex at the shell boundary]** → Keep the state minimal and repo-native, and verify representative press/release combinations in focused input tests.
- **[Existing tests encode diagonal intent behavior]** → Update those expectations explicitly so the new control rules are the documented contract.
- **[Large frame times could overshoot multiple tiles]** → Consume delta time as repeated bounded tile-step progress so final positions still land on tile centers.
- **[Queued turning may feel less responsive than free movement]** → Use last-pressed-wins for the next step so held input still responds as soon as the current tile segment ends.

## Migration Plan

1. Update the input translation boundary to publish one active cardinal movement direction with last-pressed-wins behavior.
2. Refactor `Player` movement state around tile-centered steps and destination-tile traversability checks.
3. Keep overworld runtime, camera, and rendering integration unchanged except for consuming the new player movement behavior.
4. Update automated tests to cover the new movement contract before implementation is considered complete.

Rollback strategy: revert the player movement and input translation changes together so the runtime returns to the previous free-movement vector model.

## Open Questions

No open questions remain for proposal-level implementation. The chosen direction priority rule is last-pressed-wins, and turning is intentionally deferred until the current tile step completes.
