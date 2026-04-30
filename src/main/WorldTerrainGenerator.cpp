/**
 * @file WorldTerrainGenerator.cpp
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

#include "WorldTerrainGenerator.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace rpg
{
namespace detail
{

namespace
{

[[nodiscard]] std::size_t toIndex(const TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

[[nodiscard]] std::uint32_t hashCoordinates(const std::uint32_t seed, const int x, const int y) noexcept
{
    std::uint32_t value = seed;
    value ^= static_cast<std::uint32_t>(x * 73856093);
    value ^= static_cast<std::uint32_t>(y * 19349663);
    value ^= value >> 13U;
    value *= 1274126177U;
    value ^= value >> 16U;
    return value;
}

[[nodiscard]] TileType classifyTile(const WorldConfig& config, const int x, const int y) noexcept
{
    if (x == 0
        || y == 0
        || x == config.widthInTiles - 1
        || y == config.heightInTiles - 1)
    {
        return TileType::Water;
    }

    const std::uint32_t noise = hashCoordinates(config.seed, x, y) % 100U;

    if (noise < 12U)
    {
        return TileType::Water;
    }

    if (noise < 26U)
    {
        return TileType::Sand;
    }

    if (noise > 84U)
    {
        return TileType::Forest;
    }

    return TileType::Grass;
}

[[nodiscard]] TileCoordinates findSpawnTile(
    const std::vector<TileType>& tiles,
    const int widthInTiles,
    const int heightInTiles) noexcept
{
    const TileCoordinates center{widthInTiles / 2, heightInTiles / 2};

    for (int radius = 0; radius < std::max(widthInTiles, heightInTiles); ++radius)
    {
        const int minY = std::max(1, center.y - radius);
        const int maxY = std::min(heightInTiles - 2, center.y + radius);
        const int minX = std::max(1, center.x - radius);
        const int maxX = std::min(widthInTiles - 2, center.x + radius);

        for (int y = minY; y <= maxY; ++y)
        {
            for (int x = minX; x <= maxX; ++x)
            {
                const TileCoordinates coordinates{x, y};

                if (isTraversableTileType(tiles[toIndex(coordinates, widthInTiles)]))
                {
                    return coordinates;
                }
            }
        }
    }

    return center;
}

} // namespace

GeneratedWorldData generateWorldData(const WorldConfig& config)
{
    GeneratedWorldData worldData;
    worldData.tiles.resize(static_cast<std::size_t>(config.widthInTiles * config.heightInTiles));

    for (int y = 0; y < config.heightInTiles; ++y)
    {
        for (int x = 0; x < config.widthInTiles; ++x)
        {
            const TileCoordinates coordinates{x, y};
            worldData.tiles[toIndex(coordinates, config.widthInTiles)] = classifyTile(config, x, y);
        }
    }

    worldData.spawnTile = findSpawnTile(worldData.tiles, config.widthInTiles, config.heightInTiles);
    return worldData;
}

bool isTraversableTileType(const TileType tileType) noexcept
{
    return tileType != TileType::Water;
}

} // namespace detail
} // namespace rpg
