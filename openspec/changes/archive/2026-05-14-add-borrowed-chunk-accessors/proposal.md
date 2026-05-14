## Why

`World::getChunkMetadata()` and `World::getChunkContent()` currently return retained chunk payloads by value. That is acceptable for today's small payloads, but `ChunkContent` already owns a vector of content instances and will become progressively more expensive to copy as chunk content grows and more systems inspect it.

## What Changes

- Add borrowed world-facing chunk inspection APIs so callers that only need to read retained chunk metadata or content can do so without copying the retained payload.
- Define lifetime and invalidation rules for borrowed chunk inspection results, especially around retained chunk eviction through `updateRetentionWindow()`.
- Preserve a snapshot-style access path for callers that still need an owned copy, either by keeping the current by-value accessors or by providing an explicit copy-producing alternative.
- Update tests and specs to cover non-copying inspection behavior and the retained-chunk lifetime contract.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `world-content`: change the world-facing content query boundary so callers can inspect retained chunk content without requiring a chunk-content copy on every read.
- `world-chunk-metadata`: change the world-facing metadata query boundary so callers can inspect retained metadata through the same borrowed-access pattern and documented lifetime rules.

## Impact

- Affected specs: `openspec/specs/world-content/spec.md`, `openspec/specs/world-chunk-metadata/spec.md`
- Affected code: `include/main/World.hpp`, `src/main/World.cpp`, and query-focused world tests under `tests/`
- API impact: `World` gains or changes accessor APIs for retained chunk inspection and must document the validity window for borrowed results
