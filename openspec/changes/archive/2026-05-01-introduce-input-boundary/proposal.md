## Why

The runtime already keeps most gameplay code behind repo-native types, but `Game.cpp` still polls SFML keyboard state directly to build movement intent. That leaves a backend-specific concern inside the gameplay handoff path and makes input mapping harder to test or evolve.

## What Changes

- Define an explicit input boundary between the SFML shell and overworld gameplay code.
- Translate SFML key state at the runtime edge into a repo-native overworld input type before passing it into gameplay coordination.
- Keep gameplay-facing modules and orchestration free of SFML input types and polling details.
- Add tests that exercise the input translation and confirm gameplay consumes repo-native input only.

## Capabilities

### New Capabilities
- `input-boundary`: Define how runtime-specific input polling is translated into repo-native overworld input before gameplay update logic consumes it.

### Modified Capabilities

## Impact

- Affected code: `src/main/Game.cpp`, overworld input/runtime support code, and focused tests around runtime input handling.
- Affected systems: executable runtime shell, overworld gameplay handoff, and test coverage for input mapping.
- Dependencies: no new external dependencies; continues using the existing SFML-backed shell.
