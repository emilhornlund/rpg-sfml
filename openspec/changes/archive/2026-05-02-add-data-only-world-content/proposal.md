## Why

The current world content model only exposes a minimal record of chunk ownership, content type, and identifier. As the overworld grows toward richer environmental content, the world-facing content boundary needs a stronger data model that can describe where content exists and how it should appear without introducing gameplay interaction logic yet.

## What Changes

- Expand the world content data model from simple chunk-scoped records into explicit data-only content structs.
- Add repo-native content fields for world position, footprint, and appearance selection so future systems can inspect retained content without re-deriving spatial or visual data.
- Keep the first pass intentionally non-interactive by excluding trigger logic, behavior scripts, inventory payloads, or other gameplay action state.
- Preserve deterministic chunk content generation and world-facing chunk content queries while returning the richer content shape.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `world-content`: change retained chunk content requirements so world-facing content queries return structured data-only content instances with deterministic spatial and appearance data.

## Impact

- Affected specs: `openspec/specs/world-content/spec.md`
- Affected code: `include/main/World.hpp`, `src/main/WorldContent.hpp`, `src/main/WorldContent.cpp`, `src/main/World.cpp`
- Affected tests: chunk content and vertical slice tests that currently compare the minimal `WorldContentRecord` shape
