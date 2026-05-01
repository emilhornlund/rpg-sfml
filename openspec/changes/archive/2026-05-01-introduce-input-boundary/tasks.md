## 1. Define the runtime input boundary

- [x] 1.1 Add or extract a repo-native overworld input translation helper at the SFML shell boundary.
- [x] 1.2 Ensure backend-specific key polling remains outside gameplay-facing overworld coordination and module code.

## 2. Wire translated input through the runtime

- [x] 2.1 Update the game runtime flow to construct per-frame `OverworldInput` through the boundary helper before overworld updates.
- [x] 2.2 Keep `OverworldRuntime` and `Player` consuming repo-native input types without introducing SFML dependencies.

## 3. Cover the boundary with focused tests

- [x] 3.1 Add or update tests that verify translated repo-native movement input for representative directional key combinations.
- [x] 3.2 Update runtime/gameplay-facing tests as needed to confirm gameplay continues to consume repo-native input only.
