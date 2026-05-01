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

#include "BiomeSampler.hpp"
#include "WorldTerrainGenerator.hpp"

#include <main/World.hpp>

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace
{

[[nodiscard]] std::size_t toIndex(const rpg::TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

[[nodiscard]] bool areEqual(const rpg::ChunkCandidate& lhs, const rpg::ChunkCandidate& rhs) noexcept
{
    return lhs.coordinates.x == rhs.coordinates.x
        && lhs.coordinates.y == rhs.coordinates.y
        && lhs.type == rhs.type;
}

[[nodiscard]] bool areEqual(const rpg::ChunkMetadata& lhs, const rpg::ChunkMetadata& rhs) noexcept
{
    if (lhs.chunkCoordinates.x != rhs.chunkCoordinates.x
        || lhs.chunkCoordinates.y != rhs.chunkCoordinates.y
        || lhs.biomeSummary.dominantTileType != rhs.biomeSummary.dominantTileType
        || lhs.biomeSummary.waterTileCount != rhs.biomeSummary.waterTileCount
        || lhs.biomeSummary.sandTileCount != rhs.biomeSummary.sandTileCount
        || lhs.biomeSummary.grassTileCount != rhs.biomeSummary.grassTileCount
        || lhs.biomeSummary.forestTileCount != rhs.biomeSummary.forestTileCount
        || lhs.traversabilitySummary.traversableTileCount != rhs.traversabilitySummary.traversableTileCount
        || lhs.traversabilitySummary.blockedTileCount != rhs.traversabilitySummary.blockedTileCount
        || lhs.candidates.size() != rhs.candidates.size())
    {
        return false;
    }

    for (std::size_t index = 0; index < lhs.candidates.size(); ++index)
    {
        if (!areEqual(lhs.candidates[index], rhs.candidates[index]))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool isInBounds(const rpg::TileCoordinates& coordinates, const rpg::WorldConfig& config) noexcept
{
    return coordinates.x >= 0
        && coordinates.x < config.widthInTiles
        && coordinates.y >= 0
        && coordinates.y < config.heightInTiles;
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
    const rpg::detail::TerrainGenerator terrainGenerator{config};
    const rpg::detail::GeneratedWorldData firstWorld = terrainGenerator.generateWorld();
    const rpg::detail::GeneratedWorldData secondWorld = terrainGenerator.generateWorld();

    return firstWorld.spawnTile.x == secondWorld.spawnTile.x
        && firstWorld.spawnTile.y == secondWorld.spawnTile.y
        && firstWorld.tiles == secondWorld.tiles;
}

[[nodiscard]] bool verifyDeterministicBiomeSampling()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::detail::BiomeSampler firstSampler{config};
    const rpg::detail::BiomeSampler secondSampler{config};
    const std::array<rpg::TileCoordinates, 5> sampleCoordinates = {{
        {-24, -12},
        {-1, 0},
        {0, 0},
        {16, 8},
        {72, 40},
    }};

    for (const rpg::TileCoordinates& coordinates : sampleCoordinates)
    {
        if (firstSampler.sampleTileType(coordinates.x, coordinates.y)
            != secondSampler.sampleTileType(coordinates.x, coordinates.y))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool verifyDeterministicChunkGeneration()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::detail::TerrainGenerator terrainGenerator{config};
    const rpg::detail::GeneratedChunkData firstChunk = terrainGenerator.generateChunk(1, -2);
    const rpg::detail::GeneratedChunkData secondChunk = terrainGenerator.generateChunk(1, -2);

    return firstChunk.chunkX == secondChunk.chunkX
        && firstChunk.chunkY == secondChunk.chunkY
        && firstChunk.tiles == secondChunk.tiles
        && areEqual(firstChunk.metadata, secondChunk.metadata);
}

[[nodiscard]] bool verifyOriginAnchoredSpawnSelection()
{
    const rpg::WorldConfig smallerWorldConfig{
        .seed = 0x13572468U,
        .widthInTiles = 36,
        .heightInTiles = 20,
        .tileSize = 20.0F};
    const rpg::WorldConfig largerWorldConfig{
        .seed = smallerWorldConfig.seed,
        .widthInTiles = 96,
        .heightInTiles = 72,
        .tileSize = smallerWorldConfig.tileSize};
    const rpg::World smallerWorld(smallerWorldConfig);
    const rpg::World largerWorld(largerWorldConfig);

    return smallerWorld.getSpawnTile().x == largerWorld.getSpawnTile().x
        && smallerWorld.getSpawnTile().y == largerWorld.getSpawnTile().y;
}

[[nodiscard]] bool verifySpawnValidity()
{
    rpg::World world({.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F});
    const rpg::TileCoordinates spawnTile = world.getSpawnTile();

    return world.isTraversable(spawnTile)
        && world.getTileCoordinates(world.getSpawnPosition()).x == spawnTile.x
        && world.getTileCoordinates(world.getSpawnPosition()).y == spawnTile.y;
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
        const rpg::detail::TerrainGenerator terrainGenerator{config};
        const rpg::detail::GeneratedWorldData worldData = terrainGenerator.generateWorld();

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

[[nodiscard]] bool verifyChunkGenerationExtendsBeyondInitialArea()
{
    const rpg::WorldConfig config{.seed = 0x2468ACE0U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::TileCoordinates farCoordinates{
        rpg::detail::getChunkSizeInTiles() * 8 + 1,
        rpg::detail::getChunkSizeInTiles() * 6 + 2};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();

    (void)world.getTileType(farCoordinates);
    const std::size_t generatedAfterFirstFarQuery = rpg::detail::getGeneratedChunkCount();

    if (generatedAfterFirstFarQuery <= generatedAfterConstruction)
    {
        return false;
    }

    (void)world.getTileType({farCoordinates.x + 1, farCoordinates.y + 1});
    (void)world.getTileType(farCoordinates);

    return rpg::detail::getGeneratedChunkCount() == generatedAfterFirstFarQuery;
}

[[nodiscard]] bool verifyVisibleRenderQueriesGenerateAndReuseWorldCache()
{
    const rpg::WorldConfig config{.seed = 0x0BADCAFEU, .widthInTiles = 48, .heightInTiles = 32, .tileSize = 24.0F};
    const rpg::TileCoordinates farTile{
        rpg::detail::getChunkSizeInTiles() * 10,
        rpg::detail::getChunkSizeInTiles() * 8};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();
    const rpg::ViewFrame frame{
        world.getTileCenter(farTile),
        {config.tileSize * 8.0F, config.tileSize * 6.0F}};

    const std::vector<rpg::VisibleWorldTile> firstVisibleTiles = world.getVisibleTiles(frame);
    const std::size_t generatedAfterFirstFrame = rpg::detail::getGeneratedChunkCount();

    if (firstVisibleTiles.empty() || generatedAfterFirstFrame <= generatedAfterConstruction)
    {
        return false;
    }

    (void)world.getVisibleTiles(frame);

    return rpg::detail::getGeneratedChunkCount() == generatedAfterFirstFrame;
}

[[nodiscard]] bool verifyMetadataQueriesGenerateAndReuseWorldCache()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::ChunkCoordinates farChunk{9, -7};
    const rpg::TileCoordinates tileInFarChunk{
        farChunk.x * rpg::detail::getChunkSizeInTiles() + 3,
        farChunk.y * rpg::detail::getChunkSizeInTiles() + 5};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();

    const rpg::ChunkMetadata firstMetadata = world.getChunkMetadata(farChunk);
    const std::size_t generatedAfterMetadataQuery = rpg::detail::getGeneratedChunkCount();

    if (generatedAfterMetadataQuery <= generatedAfterConstruction
        || firstMetadata.chunkCoordinates.x != farChunk.x
        || firstMetadata.chunkCoordinates.y != farChunk.y)
    {
        return false;
    }

    (void)world.getTileType(tileInFarChunk);
    const rpg::ChunkMetadata secondMetadata = world.getChunkMetadata(tileInFarChunk);

    return rpg::detail::getGeneratedChunkCount() == generatedAfterMetadataQuery
        && areEqual(firstMetadata, secondMetadata);
}

[[nodiscard]] bool verifyChunkMetadataCandidates()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    std::optional<rpg::ChunkMetadata> traversableChunk;
    std::optional<rpg::ChunkMetadata> blockedChunk;

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkMetadata metadata = world.getChunkMetadata(rpg::ChunkCoordinates{chunkX, chunkY});

            if (!traversableChunk.has_value() && metadata.traversabilitySummary.traversableTileCount > 0)
            {
                traversableChunk = metadata;
            }

            if (!blockedChunk.has_value() && metadata.traversabilitySummary.traversableTileCount == 0)
            {
                blockedChunk = metadata;
            }

            if (traversableChunk.has_value() && blockedChunk.has_value())
            {
                break;
            }
        }

        if (traversableChunk.has_value() && blockedChunk.has_value())
        {
            break;
        }
    }

    if (!traversableChunk.has_value() || !blockedChunk.has_value())
    {
        return false;
    }

    if (!blockedChunk->candidates.empty())
    {
        return false;
    }

    if (traversableChunk->candidates.empty())
    {
        return false;
    }

    for (const rpg::ChunkCandidate& candidate : traversableChunk->candidates)
    {
        if (!world.isTraversable(candidate.coordinates))
        {
            return false;
        }
    }

    return true;
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
        {80, 64},
        {96, 64},
        {-32, 48},
        {120, -24},
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

    if (!verifyDeterministicBiomeSampling())
    {
        return 1;
    }

    if (!verifyDeterministicChunkGeneration())
    {
        return 1;
    }

    if (!verifyOriginAnchoredSpawnSelection())
    {
        return 1;
    }

    if (!verifySpawnValidity())
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

    if (!verifyChunkGenerationExtendsBeyondInitialArea())
    {
        return 1;
    }

    if (!verifyVisibleRenderQueriesGenerateAndReuseWorldCache())
    {
        return 1;
    }

    if (!verifyMetadataQueriesGenerateAndReuseWorldCache())
    {
        return 1;
    }

    if (!verifyChunkMetadataCandidates())
    {
        return 1;
    }

    if (!verifyAbsoluteCoordinateSignalsAreWorldSizeIndependent())
    {
        return 1;
    }

    return 0;
}
