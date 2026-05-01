## Why

The procedural world already produces stable terrain, but each retained chunk is still effectively just a bag of colored tiles. Adding lightweight chunk-level metadata now gives future gameplay systems a world-facing substrate for biome-aware spawning, navigation, encounter seeding, and point-of-interest placement without committing to a full ECS or persistence design too early.

## What Changes

- Add deterministic chunk-level metadata that is generated and retained alongside chunk tile content.
- Define metadata summaries for dominant biome signals, traversability, and spawn or point-of-interest candidates within each chunk.
- Expose world-facing access to retained chunk metadata so future gameplay systems can query chunk summaries without re-deriving them from raw tiles.
- Keep the scope intentionally narrow: no persistence format, no entity simulation layer, and no full gameplay systems built on top of the metadata yet.

## Capabilities

### New Capabilities
- `world-chunk-metadata`: Define the deterministic metadata generated for each retained chunk, including biome summaries, traversability summaries, and spawn or point-of-interest candidates.

### Modified Capabilities
- `world-chunk-caching`: Extend retained chunk content so cached chunk state includes reusable metadata alongside tile classification data.

## Impact

- Affected specs: `world-chunk-caching`, new `world-chunk-metadata`
- Affected code: world-owned chunk generation collaborators, retained chunk data structures, and world-facing metadata query surfaces
- No new external dependencies
- Keeps future gameplay additions anchored in world data without expanding `Game` into a gameplay rules owner
