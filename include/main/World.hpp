/**
 * @file World.hpp
 *
 * MIT License
 *
 * Copyright (c) 2026 Emil Hörnlund
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef RPG_MAIN_WORLD_HPP
#define RPG_MAIN_WORLD_HPP

#include <main/OverworldTypes.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Core runtime types for the RPG executable.
 */
namespace rpg
{

namespace detail
{
class TerrainGenerator;
class WorldContent;
} // namespace detail

/**
 * @brief Configuration inputs for deterministic overworld generation.
 *
 * The world configuration captures deterministic generation inputs for the
 * overworld. Reusing the same configuration produces the same terrain and
 * spawn behavior. Width and height remain available for compatibility and
 * finite sampling helpers, but they do not define a hard world boundary for
 * runtime terrain queries.
 */
struct WorldConfig
{
    std::uint32_t seed = 0x00C0FFEEU;
    int widthInTiles = 64;
    int heightInTiles = 64;
    float tileSize = 16.0F;
};

/**
 * @brief Tile categories for the overworld slice.
 */
enum class TileType
{
    Grass,
    Sand,
    Water,
    Forest
};

/**
 * @brief Render-facing data for a visible overworld tile.
 */
struct VisibleWorldTile
{
    TileCoordinates coordinates{0, 0};
    TileType tileType = TileType::Water;
    WorldPosition center{0.0F, 0.0F};
};

/**
 * @brief Chunk coordinates in the streaming overworld grid.
 */
struct ChunkCoordinates
{
    int x = 0;
    int y = 0;
};

/**
 * @brief Coarse biome summary for a retained chunk.
 */
struct ChunkBiomeSummary
{
    TileType dominantTileType = TileType::Water;
    int waterTileCount = 0;
    int sandTileCount = 0;
    int grassTileCount = 0;
    int forestTileCount = 0;
};

/**
 * @brief Coarse traversability summary for a retained chunk.
 */
struct ChunkTraversabilitySummary
{
    int traversableTileCount = 0;
    int blockedTileCount = 0;
};

/**
 * @brief Chunk-scale metadata retained alongside generated terrain tiles.
 */
struct ChunkMetadata
{
    ChunkCoordinates chunkCoordinates{0, 0};
    ChunkBiomeSummary biomeSummary;
    ChunkTraversabilitySummary traversabilitySummary;
};

/**
 * @brief Deterministic chunk-scoped content categories.
 */
enum class ContentType
{
    Tree,
    Shrub
};

/**
 * @brief Opaque appearance selector for deterministic chunk content.
 */
struct ContentAppearanceId
{
    std::uint32_t value = 0;
};

/**
 * @brief World-space footprint for deterministic chunk content.
 */
struct ContentFootprint
{
    WorldPosition offset{0.0F, 0.0F};
    WorldSize size{0.0F, 0.0F};
};

/**
 * @brief Deterministic data-only content instance retained for a chunk.
 */
struct ContentInstance
{
    std::uint64_t id = 0;
    ContentType type = ContentType::Tree;
    std::string prototypeId;
    TileCoordinates anchorTile{0, 0};
    WorldPosition position{0.0F, 0.0F};
    ContentFootprint footprint{};
    ContentAppearanceId appearanceId{};
    float sortKeyY = 0.0F;
};

/**
 * @brief Deterministic chunk-owned content retained for a generated chunk.
 */
struct ChunkContent
{
    ChunkCoordinates chunkCoordinates{0, 0};
    std::vector<ContentInstance> instances;
};

/**
 * @brief Visible generated content instance for an active camera frame.
 */
struct VisibleWorldContent
{
    ContentInstance instance;
};

/**
 * @brief Owns world-specific runtime state.
 *
 * The World class provides the dedicated boundary for world-facing gameplay
 * data and behavior. It exists so future terrain, chunk, and procedural
 * generation logic can be added behind a stable type instead of growing inside
 * the top-level game loop coordinator.
 */
class World
{
public:
    /**
     * @brief Construct a new world module.
     *
     * Prepares the world-owned state used by the runtime.
     */
    World();

    /**
     * @brief Construct a new world module from explicit generation inputs.
     *
     * @param config Deterministic configuration used to generate the world.
     */
    explicit World(const WorldConfig& config);

    /**
     * @brief Destroy the world module.
     *
     * Releases any resources retained by the world-owned state.
     */
    ~World();

    /**
     * @brief Get the size of a rendered tile in world units.
     *
     * @return Tile size in world units.
     */
    [[nodiscard]] float getTileSize() const noexcept;

    /**
     * @brief Get the deterministic world-generation seed.
     *
     * @return Seed used for world generation and seed-backed render selection.
     */
    [[nodiscard]] std::uint32_t getGenerationSeed() const noexcept;

    /**
     * @brief Get the deterministic player spawn tile.
     *
     * @return Traversable spawn coordinates inside the world grid.
     */
    [[nodiscard]] TileCoordinates getSpawnTile() const noexcept;

    /**
     * @brief Get the deterministic player spawn position.
     *
     * @return Spawn position at the center of the spawn tile.
     */
    [[nodiscard]] WorldPosition getSpawnPosition() const noexcept;

    /**
     * @brief Check whether a tile is traversable.
     *
     * @param coordinates Tile coordinates to inspect.
     * @return True when the resolved tile can be walked on.
     */
    [[nodiscard]] bool isTraversable(const TileCoordinates& coordinates) const;

    /**
     * @brief Check whether a world position is traversable.
     *
     * @param position World position to inspect.
     * @return True when the position maps to a traversable tile.
     */
    [[nodiscard]] bool isTraversable(const WorldPosition& position) const;

    /**
     * @brief Read the tile type at the given coordinates.
     *
     * @param coordinates Tile coordinates to inspect.
     * @return Tile type resolved for the coordinates.
     */
    [[nodiscard]] TileType getTileType(const TileCoordinates& coordinates) const;

    /**
     * @brief Convert a tile coordinate to the chunk that owns it.
     *
     * @param coordinates Absolute tile coordinates.
     * @return Chunk coordinates that own the tile.
     */
    [[nodiscard]] ChunkCoordinates getChunkCoordinates(const TileCoordinates& coordinates) const noexcept;

    /**
     * @brief Read retained metadata for a generated chunk.
     *
     * Missing chunks are generated on demand and retained before the metadata
     * is returned.
     *
     * @param coordinates Chunk coordinates to inspect.
     * @return Retained metadata for the chunk.
     */
    [[nodiscard]] ChunkMetadata getChunkMetadata(const ChunkCoordinates& coordinates) const;

    /**
     * @brief Read retained metadata for the chunk that owns a tile.
     *
     * @param coordinates Tile coordinates used to resolve the owning chunk.
     * @return Retained metadata for the owning chunk.
     */
    [[nodiscard]] ChunkMetadata getChunkMetadata(const TileCoordinates& coordinates) const;

    /**
     * @brief Read retained content for a generated chunk.
     *
     * Missing chunks are generated on demand and retained before the content is
     * returned.
     *
     * @param coordinates Chunk coordinates to inspect.
     * @return Deterministic retained content for the chunk.
     */
    [[nodiscard]] ChunkContent getChunkContent(const ChunkCoordinates& coordinates) const;

    /**
     * @brief Read retained content for the chunk that owns a tile.
     *
     * @param coordinates Tile coordinates used to resolve the owning chunk.
     * @return Deterministic retained content for the owning chunk.
     */
    [[nodiscard]] ChunkContent getChunkContent(const TileCoordinates& coordinates) const;

    /**
     * @brief Enumerate the visible world tiles for a camera frame.
     *
     * The query derives visible tile and chunk bounds from the frame, applies a
     * bounded overscan margin, ensures the intersecting chunks exist, and
     * serves tiles from retained chunk data.
     *
     * @param frame Camera frame used to determine visible terrain.
     * @return Render-facing data for the visible world tiles.
     */
    [[nodiscard]] std::vector<VisibleWorldTile> getVisibleTiles(const ViewFrame& frame) const;

    /**
     * @brief Enumerate visible generated content for a camera frame.
     *
     * The query derives visible chunk bounds from the frame, ensures the
     * intersecting chunks exist, and returns retained content instances whose
     * world-space footprints intersect the active camera frame.
     *
     * @param frame Camera frame used to determine visible generated content.
     * @return Visible generated content instances for the frame.
     */
    [[nodiscard]] std::vector<VisibleWorldContent> getVisibleContent(const ViewFrame& frame) const;

    /**
     * @brief Count retained generated-content instances across loaded chunks.
     *
     * @return Number of generated-content instances currently retained in the
     * loaded chunk cache.
     */
    [[nodiscard]] std::size_t getRetainedGeneratedContentCount() const noexcept;

    /**
     * @brief Count retained chunks in the active world cache.
     *
     * @return Number of chunks currently retained by the world.
     */
    [[nodiscard]] std::size_t getRetainedChunkCount() const noexcept;

    /**
     * @brief Update the active retention window used for chunk eviction.
     *
     * The world derives a chunk retention window from the supplied frame and
     * prunes retained chunks that fall outside the buffered retention bounds.
     *
     * @param frame Camera frame that defines the active streaming window.
     */
    void updateRetentionWindow(const ViewFrame& frame);

    /**
     * @brief Convert a tile coordinate to the center of that tile in world space.
     *
     * @param coordinates Tile coordinates to convert.
     * @return Center point of the tile in world-space units.
     */
    [[nodiscard]] WorldPosition getTileCenter(const TileCoordinates& coordinates) const noexcept;

    /**
     * @brief Convert a world position to tile coordinates.
     *
     * @param position World position to convert.
     * @return Tile coordinates containing the position.
     */
    [[nodiscard]] TileCoordinates getTileCoordinates(const WorldPosition& position) const noexcept;

private:
    /**
     * @brief Internal world state container.
     *
     * The nested state keeps the public type ready for future internal
     * expansion without exposing implementation details in the header.
     */
    struct State
    {
        struct RetainedChunkData
        {
            std::vector<TileType> tiles;
            ChunkMetadata metadata;
            ChunkContent content;
        };

        struct RetentionWindow
        {
            int minChunkX = 0;
            int maxChunkX = -1;
            int minChunkY = 0;
            int maxChunkY = -1;
            bool isActive = false;
        };

        WorldConfig config;
        TileCoordinates spawnTile{0, 0};
        std::shared_ptr<const detail::TerrainGenerator> terrainGenerator;
        std::shared_ptr<const detail::WorldContent> worldContent;
        mutable std::map<std::pair<int, int>, RetainedChunkData> chunks;
        mutable std::size_t retainedGeneratedContentCount = 0;
        mutable RetentionWindow retentionWindow;
    };

    /**
     * @brief Storage for world-owned runtime state.
     */
    State m_state;

    [[nodiscard]] State::RetainedChunkData& ensureChunkRetained(const ChunkCoordinates& coordinates) const;
    void pruneRetainedChunks() const;
};

} // namespace rpg

#endif // RPG_MAIN_WORLD_HPP
