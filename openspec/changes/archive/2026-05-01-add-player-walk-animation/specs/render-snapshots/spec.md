## MODIFIED Requirements

### Requirement: Snapshot entries are render-ready without gameplay reinterpretation
Each visible tile entry and marker entry in the render snapshot SHALL provide the world-space geometry and presentation identifiers needed for drawing so the outer game shell can render the frame without re-deriving gameplay presentation rules from simulation state. Player-facing snapshot data SHALL include the placement and animation selectors needed to choose the correct player sprite frame from the bundled walking spritesheet without requiring the game shell to infer facing or movement presentation from raw gameplay state.

#### Scenario: Game renders directly from snapshot entries
- **WHEN** the outer game shell renders an overworld frame from the published snapshot
- **THEN** it uses the snapshot's tile entries and marker entries as the source of draw data
- **AND** it does not infer additional overworld presentation state by inspecting gameplay-owned modules
- **AND** it can choose the correct player spritesheet frame from snapshot-provided player presentation metadata
