## Tasks

- [x] Add a dedicated `WorldContent` boundary and chunk content record types that deterministically derive content from world seed, chunk coordinates, and chunk metadata.
- [x] Update `World` retained chunk storage and world-facing query APIs so tiles, metadata, and content are generated and cached together per chunk.
- [x] Remove candidate-location generation from `ChunkMetadata` and `TerrainGenerator`, keeping metadata focused on biome and traversability summaries.
- [x] Update and extend tests to cover deterministic chunk content queries, retained chunk reuse for content, and the revised metadata contract.
