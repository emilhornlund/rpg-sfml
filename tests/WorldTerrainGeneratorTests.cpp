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

#include <main/World.hpp>

#include <array>
#include <cstddef>
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

[[nodiscard]] bool hasAdjacentTileType(
    const std::vector<rpg::TileType>& tiles,
    const rpg::TileCoordinates& coordinates,
    const rpg::WorldConfig& config,
    const rpg::TileType tileType) noexcept
{
    constexpr std::array<rpg::TileCoordinates, 4> kOffsets = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    for (const rpg::TileCoordinates& offset : kOffsets)
    {
        const rpg::TileCoordinates candidate{coordinates.x + offset.x, coordinates.y + offset.y};

        if (isInBounds(candidate, config)
            && tiles[toIndex(candidate, config.widthInTiles)] == tileType)
        {
            return true;
        }
    }

    return false;
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

[[nodiscard]] bool verifyDeterministicChunkGeneration()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::detail::GeneratedChunkData firstChunk = rpg::detail::generateChunkData(config, 1, 0);
    const rpg::detail::GeneratedChunkData secondChunk = rpg::detail::generateChunkData(config, 1, 0);

    return firstChunk.chunkX == secondChunk.chunkX
        && firstChunk.chunkY == secondChunk.chunkY
        && firstChunk.tiles == secondChunk.tiles;
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

[[nodiscard]] bool verifyCoastlinesAndInlandBiomes()
{
    const std::array<rpg::WorldConfig, 3> configs = {{
        {.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F},
        {.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F},
        {.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F},
    }};

    bool foundShorelineSand = false;
    bool foundInlandGrass = false;
    bool foundInlandForest = false;

    for (const rpg::WorldConfig& config : configs)
    {
        const rpg::detail::GeneratedWorldData worldData = rpg::detail::generateWorldData(config);

        for (int y = 1; y < config.heightInTiles - 1; ++y)
        {
            for (int x = 1; x < config.widthInTiles - 1; ++x)
            {
                const rpg::TileCoordinates coordinates{x, y};
                const rpg::TileType tileType = worldData.tiles[toIndex(coordinates, config.widthInTiles)];
                const bool adjacentToWater = hasAdjacentTileType(worldData.tiles, coordinates, config, rpg::TileType::Water);
                const bool isInland = x > 2
                    && x < config.widthInTiles - 3
                    && y > 2
                    && y < config.heightInTiles - 3
                    && !adjacentToWater;

                if (tileType == rpg::TileType::Sand && adjacentToWater)
                {
                    foundShorelineSand = true;
                }

                if (!isInland)
                {
                    continue;
                }

                if (tileType == rpg::TileType::Grass)
                {
                    foundInlandGrass = true;
                }
                else if (tileType == rpg::TileType::Forest)
                {
                    foundInlandForest = true;
                }
            }
        }
    }

    return foundShorelineSand && foundInlandGrass && foundInlandForest;
}

[[nodiscard]] bool verifyNegativeChunkCoordinateMapping()
{
    const rpg::TileCoordinates leftOfOrigin{
        rpg::detail::getChunkLocalCoordinate(-1),
        rpg::detail::getChunkLocalCoordinate(-17)};
    const rpg::TileCoordinates recoveredLeftOfOrigin = rpg::detail::getWorldTileCoordinates(-1, -2, leftOfOrigin);
    const rpg::TileCoordinates origin{
        rpg::detail::getChunkLocalCoordinate(0),
        rpg::detail::getChunkLocalCoordinate(16)};
    const rpg::TileCoordinates recoveredOrigin = rpg::detail::getWorldTileCoordinates(0, 1, origin);

    return rpg::detail::getChunkCoordinate(-1) == -1
        && rpg::detail::getChunkCoordinate(-16) == -1
        && rpg::detail::getChunkCoordinate(-17) == -2
        && rpg::detail::getChunkCoordinate(15) == 0
        && rpg::detail::getChunkCoordinate(16) == 1
        && leftOfOrigin.x == rpg::detail::getChunkSizeInTiles() - 1
        && leftOfOrigin.y == rpg::detail::getChunkSizeInTiles() - 1
        && recoveredLeftOfOrigin.x == -1
        && recoveredLeftOfOrigin.y == -17
        && origin.x == 0
        && origin.y == 0
        && recoveredOrigin.x == 0
        && recoveredOrigin.y == 16;
}

[[nodiscard]] std::size_t expectedGeneratedChunkCount(const rpg::WorldConfig& config) noexcept
{
    const int chunkCountX = rpg::detail::getChunkCoordinate(config.widthInTiles - 1) + 1;
    const int chunkCountY = rpg::detail::getChunkCoordinate(config.heightInTiles - 1) + 1;
    return static_cast<std::size_t>(chunkCountX * chunkCountY);
}

[[nodiscard]] bool verifyChunkReuseThroughWorldCache()
{
    const rpg::WorldConfig config{.seed = 0x2468ACE0U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedChunkCount = rpg::detail::getGeneratedChunkCount();

    if (generatedChunkCount != expectedGeneratedChunkCount(config))
    {
        return false;
    }

    (void)world.getTileType({0, 0});
    (void)world.getTileType({1, 1});
    (void)world.getTileType({15, 15});
    (void)world.getTileType({16, 0});
    (void)world.getTileType({39, 23});
    (void)world.getTileType({-1, 0});

    return rpg::detail::getGeneratedChunkCount() == generatedChunkCount;
}

[[nodiscard]] bool verifyVisibleRenderQueriesReuseWorldCache()
{
    const rpg::WorldConfig config{.seed = 0x0BADCAFEU, .widthInTiles = 48, .heightInTiles = 32, .tileSize = 24.0F};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedChunkCount = rpg::detail::getGeneratedChunkCount();
    const rpg::WorldPosition spawnPosition = world.getSpawnPosition();
    const rpg::ViewFrame frame{spawnPosition, {config.tileSize * 8.0F, config.tileSize * 6.0F}};

    if (generatedChunkCount != expectedGeneratedChunkCount(config))
    {
        return false;
    }

    (void)world.getVisibleTiles(frame);
    (void)world.getVisibleTiles(frame);

    return rpg::detail::getGeneratedChunkCount() == generatedChunkCount;
}

[[nodiscard]] bool verifyAbsoluteCoordinateSignalsAreWorldSizeIndependent()
{
    const rpg::WorldConfig smallerWorldConfig{
        .seed = 0x55AA12EFU,
        .widthInTiles = 40,
        .heightInTiles = 24,
        .tileSize = 24.0F};
    const rpg::WorldConfig largerWorldConfig{
        .seed = smallerWorldConfig.seed,
        .widthInTiles = 80,
        .heightInTiles = 64,
        .tileSize = smallerWorldConfig.tileSize};
    const std::array<rpg::TileCoordinates, 4> sampleCoordinates = {{
        {16, 10},
        {20, 12},
        {24, 11},
        {28, 12},
    }};

    const rpg::World smallerWorld(smallerWorldConfig);
    const rpg::World largerWorld(largerWorldConfig);

    for (const rpg::TileCoordinates& coordinates : sampleCoordinates)
    {
        if (smallerWorld.getTileType(coordinates) != largerWorld.getTileType(coordinates))
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

    if (!verifyDeterministicChunkGeneration())
    {
        return 1;
    }

    if (!verifyBorderAndInteriorTraversal())
    {
        return 1;
    }

    if (!verifyCoastlinesAndInlandBiomes())
    {
        return 1;
    }

    if (!verifyNegativeChunkCoordinateMapping())
    {
        return 1;
    }

    if (!verifyChunkReuseThroughWorldCache())
    {
        return 1;
    }

    if (!verifyVisibleRenderQueriesReuseWorldCache())
    {
        return 1;
    }

    if (!verifyAbsoluteCoordinateSignalsAreWorldSizeIndependent())
    {
        return 1;
    }

    return 0;
}
