/**
 * @file WorldTerrainGeneratorTests.cpp
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

#include <vector>

namespace
{

[[nodiscard]] std::size_t toIndex(const rpg::TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

[[nodiscard]] bool isInBounds(const rpg::TileCoordinates& coordinates, const rpg::WorldConfig& config) noexcept
{
    return coordinates.x >= 0
        && coordinates.x < config.widthInTiles
        && coordinates.y >= 0
        && coordinates.y < config.heightInTiles;
}

[[nodiscard]] bool isTraversable(
    const std::vector<rpg::TileType>& tiles,
    const rpg::TileCoordinates& coordinates,
    const int widthInTiles) noexcept
{
    return rpg::detail::isTraversableTileType(tiles[toIndex(coordinates, widthInTiles)]);
}

[[nodiscard]] bool verifyDeterministicGeneration()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::detail::GeneratedWorldData firstWorld = rpg::detail::generateWorldData(config);
    const rpg::detail::GeneratedWorldData secondWorld = rpg::detail::generateWorldData(config);

    return firstWorld.spawnTile.x == secondWorld.spawnTile.x
        && firstWorld.spawnTile.y == secondWorld.spawnTile.y
        && firstWorld.tiles == secondWorld.tiles;
}

[[nodiscard]] bool verifySpawnValidity()
{
    const rpg::WorldConfig config{.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F};
    const rpg::detail::GeneratedWorldData worldData = rpg::detail::generateWorldData(config);

    return isInBounds(worldData.spawnTile, config)
        && isTraversable(worldData.tiles, worldData.spawnTile, config.widthInTiles);
}

[[nodiscard]] bool verifyBorderAndInteriorTraversal()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 24, .heightInTiles = 18, .tileSize = 16.0F};
    const rpg::detail::GeneratedWorldData worldData = rpg::detail::generateWorldData(config);

    for (int x = 0; x < config.widthInTiles; ++x)
    {
        if (worldData.tiles[toIndex({x, 0}, config.widthInTiles)] != rpg::TileType::Water
            || worldData.tiles[toIndex({x, config.heightInTiles - 1}, config.widthInTiles)] != rpg::TileType::Water)
        {
            return false;
        }
    }

    for (int y = 0; y < config.heightInTiles; ++y)
    {
        if (worldData.tiles[toIndex({0, y}, config.widthInTiles)] != rpg::TileType::Water
            || worldData.tiles[toIndex({config.widthInTiles - 1, y}, config.widthInTiles)] != rpg::TileType::Water)
        {
            return false;
        }
    }

    for (const rpg::TileType tileType : worldData.tiles)
    {
        if (tileType != rpg::TileType::Water && rpg::detail::isTraversableTileType(tileType) == false)
        {
            return false;
        }
    }

    return true;
}

} // namespace

int main()
{
    if (!verifyDeterministicGeneration())
    {
        return 1;
    }

    if (!verifySpawnValidity())
    {
        return 1;
    }

    if (!verifyBorderAndInteriorTraversal())
    {
        return 1;
    }

    return 0;
}
