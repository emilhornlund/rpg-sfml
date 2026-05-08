## Why

The overworld vegetation runtime currently uses hardcoded placement pools and terrain checks, so classification metadata such as `placeOn` and `biomes` is not respected. This prevents intended assets like water lilies and marsh reeds from appearing on valid water tiles and makes the staged vegetation classification less trustworthy as the source of placement behavior.

## What Changes

- Update vegetation placement so prototype eligibility is derived from staged vegetation classification metadata instead of only hardcoded forest and grass prototype lists.
- Respect per-prototype `placeOn` tile constraints when evaluating anchor tiles for deterministic vegetation placement.
- Respect per-prototype `biomes` weights when determining whether a prototype may appear on a valid anchor tile.
- Preserve the current deterministic, data-only world content model and keep shoreline-specific placement rules out of scope for this change.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `vegetation-content`: vegetation placement must honor classification-driven tile placement constraints and biome weighting for deterministic prototype selection.
- `tileset-asset-loading`: staged vegetation classification loading must retain the placement metadata needed by runtime vegetation placement.

## Impact

- Affected specs: `openspec/specs/vegetation-content/spec.md`, `openspec/specs/tileset-asset-loading/spec.md`
- Affected code: vegetation metadata loading, vegetation atlas support, world content generation, and vegetation-related tests
- No expected external API changes; runtime behavior changes by allowing metadata-qualified vegetation such as water lilies and marsh reeds to appear
