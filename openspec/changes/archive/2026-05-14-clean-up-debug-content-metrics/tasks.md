## 1. Snapshot and overlay cleanup

- [x] 1.1 Remove the redundant rendered generated-content field from the overworld debug snapshot and stop populating it in `OverworldRuntime`.
- [x] 1.2 Update debug overlay formatting to present only the visible generated-content metric and keep the remaining diagnostics unchanged.

## 2. Coverage and spec alignment

- [x] 2.1 Update runtime and overlay-string tests to match the collapsed generated-content metric set.
- [x] 2.2 Verify the implementation matches the debug-overlay-display delta spec and does not reintroduce a duplicate generated-content metric.
