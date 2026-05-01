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

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

/**
 * @brief Core runtime types for the RPG executable.
 */
namespace rpg
{

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
 * @brief Deterministic chunk candidate categories for future gameplay systems.
 */
enum class ChunkCandidateType
{
    Spawn,
    PointOfInterest
};

/**
 * @brief Deterministic candidate location retained for a chunk.
 */
struct ChunkCandidate
{
    TileCoordinates coordinates{0, 0};
    ChunkCandidateType type = ChunkCandidateType::Spawn;
};

/**
 * @brief Chunk-scale metadata retained alongside generated terrain tiles.
 */
struct ChunkMetadata
{
    ChunkCoordinates chunkCoordinates{0, 0};
    ChunkBiomeSummary biomeSummary;
    ChunkTraversabilitySummary traversabilitySummary;
    std::vector<ChunkCandidate> candidates;
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
        };

        WorldConfig config;
        TileCoordinates spawnTile{0, 0};
        mutable std::map<std::pair<int, int>, RetainedChunkData> chunks;
    };

    /**
     * @brief Storage for world-owned runtime state.
     */
    State m_state;

    [[nodiscard]] State::RetainedChunkData& ensureChunkRetained(const ChunkCoordinates& coordinates) const;
};

} // namespace rpg

#endif // RPG_MAIN_WORLD_HPP
