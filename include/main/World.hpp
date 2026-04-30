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
 * The world configuration captures the generation inputs that define a finite
 * overworld slice. Reusing the same configuration produces the same tile
 * layout, spawn point, dimensions, and tile size.
 */
struct WorldConfig
{
    std::uint32_t seed = 0x00C0FFEEU;
    int widthInTiles = 64;
    int heightInTiles = 64;
    float tileSize = 32.0F;
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
     * @brief Get the tile-grid width.
     *
     * @return Number of tiles across the generated world.
     */
    [[nodiscard]] int getWidthInTiles() const noexcept;

    /**
     * @brief Get the tile-grid height.
     *
     * @return Number of tiles down the generated world.
     */
    [[nodiscard]] int getHeightInTiles() const noexcept;

    /**
     * @brief Get the size of a rendered tile in world units.
     *
     * @return Tile size in world units.
     */
    [[nodiscard]] float getTileSize() const noexcept;

    /**
     * @brief Get the total world size in world units.
     *
     * @return The width and height of the generated world.
     */
    [[nodiscard]] WorldSize getWorldSize() const noexcept;

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
     * @brief Check whether tile coordinates are inside the world.
     *
     * @param coordinates Tile coordinates to inspect.
     * @return True when the coordinates are inside the generated grid.
     */
    [[nodiscard]] bool isInBounds(const TileCoordinates& coordinates) const noexcept;

    /**
     * @brief Check whether a tile is traversable.
     *
     * @param coordinates Tile coordinates to inspect.
     * @return True when the tile exists and can be walked on.
     */
    [[nodiscard]] bool isTraversable(const TileCoordinates& coordinates) const noexcept;

    /**
     * @brief Check whether a world position is traversable.
     *
     * @param position World position to inspect.
     * @return True when the position maps to a valid traversable tile.
     */
    [[nodiscard]] bool isTraversable(const WorldPosition& position) const noexcept;

    /**
     * @brief Read the tile type at the given coordinates.
     *
     * @param coordinates Tile coordinates to inspect.
     * @return Tile type at the coordinates, or water when out of bounds.
     */
    [[nodiscard]] TileType getTileType(const TileCoordinates& coordinates) const noexcept;

    /**
     * @brief Enumerate the visible world tiles for a camera frame.
     *
     * The query derives visible tile and chunk bounds from the frame, applies a
     * bounded overscan margin, clips the result to valid world coordinates, and
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
        WorldConfig config;
        TileCoordinates spawnTile{0, 0};
        std::map<std::pair<int, int>, std::vector<TileType>> chunks;
    };

    /**
     * @brief Storage for world-owned runtime state.
     */
    State m_state;
};

} // namespace rpg

#endif // RPG_MAIN_WORLD_HPP
