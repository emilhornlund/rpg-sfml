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
#include "GameAssetSupport.hpp"
#include "WorldTerrainGenerator.hpp"

#include <main/World.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

namespace
{

constexpr float kFloatTolerance = 0.001F;

[[nodiscard]] std::size_t toIndex(const rpg::TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

[[nodiscard]] bool areClose(const float lhs, const float rhs) noexcept
{
    return std::fabs(lhs - rhs) < kFloatTolerance;
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
        || lhs.traversabilitySummary.blockedTileCount != rhs.traversabilitySummary.blockedTileCount)
    {
        return false;
    }

    return true;
}

[[nodiscard]] bool areEqual(const rpg::ContentInstance& lhs, const rpg::ContentInstance& rhs) noexcept
{
    return lhs.id == rhs.id
        && lhs.type == rhs.type
        && lhs.prototypeId == rhs.prototypeId
        && lhs.anchorTile.x == rhs.anchorTile.x
        && lhs.anchorTile.y == rhs.anchorTile.y
        && areClose(lhs.position.x, rhs.position.x)
        && areClose(lhs.position.y, rhs.position.y)
        && areClose(lhs.footprint.offset.x, rhs.footprint.offset.x)
        && areClose(lhs.footprint.offset.y, rhs.footprint.offset.y)
        && areClose(lhs.footprint.size.width, rhs.footprint.size.width)
        && areClose(lhs.footprint.size.height, rhs.footprint.size.height)
        && lhs.appearanceId.value == rhs.appearanceId.value
        && areClose(lhs.sortKeyY, rhs.sortKeyY);
}

[[nodiscard]] bool areEqual(
    const std::vector<rpg::ContentInstance>& lhs,
    const std::vector<rpg::ContentInstance>& rhs) noexcept
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    for (std::size_t index = 0; index < lhs.size(); ++index)
    {
        if (!areEqual(lhs[index], rhs[index]))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool areEqual(const rpg::ChunkContent& lhs, const rpg::ChunkContent& rhs) noexcept
{
    return lhs.chunkCoordinates.x == rhs.chunkCoordinates.x
        && lhs.chunkCoordinates.y == rhs.chunkCoordinates.y
        && areEqual(lhs.instances, rhs.instances);
}

[[nodiscard]] bool containsVisibleContent(
    const std::vector<rpg::VisibleWorldContent>& visibleContent,
    const rpg::ContentInstance& instance) noexcept
{
    for (const rpg::VisibleWorldContent& visibleInstance : visibleContent)
    {
        if (areEqual(visibleInstance.instance, instance))
        {
            return true;
        }
    }

    return false;
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

struct CardinalNeighborMask
{
    bool north = false;
    bool east = false;
    bool south = false;
    bool west = false;
};

[[nodiscard]] CardinalNeighborMask getMatchingCardinalNeighborMask(
    const rpg::World& world,
    const rpg::TileCoordinates& coordinates)
{
    const rpg::TileType tileType = world.getTileType(coordinates);

    return {
        world.getTileType({coordinates.x, coordinates.y - 1}) == tileType,
        world.getTileType({coordinates.x + 1, coordinates.y}) == tileType,
        world.getTileType({coordinates.x, coordinates.y + 1}) == tileType,
        world.getTileType({coordinates.x - 1, coordinates.y}) == tileType,
    };
}

[[nodiscard]] bool hasUnsupportedTerrainShape(
    const rpg::World& world,
    const rpg::TileCoordinates& coordinates)
{
    const CardinalNeighborMask mask = getMatchingCardinalNeighborMask(world, coordinates);
    const int cardinalNeighborCount =
        static_cast<int>(mask.north) + static_cast<int>(mask.east) + static_cast<int>(mask.south) + static_cast<int>(mask.west);

    return cardinalNeighborCount <= 1;
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

[[nodiscard]] bool verifyContentQueriesGenerateAndReuseWorldCache()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::ChunkCoordinates farChunk{9, -7};
    const rpg::TileCoordinates tileInFarChunk{
        farChunk.x * rpg::detail::getChunkSizeInTiles() + 3,
        farChunk.y * rpg::detail::getChunkSizeInTiles() + 5};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();

    const rpg::ChunkContent firstContent = world.getChunkContent(farChunk);
    const std::size_t generatedAfterContentQuery = rpg::detail::getGeneratedChunkCount();

    if (generatedAfterContentQuery <= generatedAfterConstruction)
    {
        return false;
    }

    (void)world.getTileType(tileInFarChunk);
    const rpg::ChunkContent secondContent = world.getChunkContent(tileInFarChunk);

    return rpg::detail::getGeneratedChunkCount() == generatedAfterContentQuery
        && areEqual(firstContent, secondContent);
}

[[nodiscard]] bool verifyChunkContentRecords()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    std::optional<rpg::ChunkContent> supportedChunk;

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkCoordinates chunkCoordinates{chunkX, chunkY};
            const rpg::ChunkMetadata metadata = world.getChunkMetadata(rpg::ChunkCoordinates{chunkX, chunkY});
            const rpg::ChunkContent content = world.getChunkContent(chunkCoordinates);

            if (!supportedChunk.has_value()
                && metadata.traversabilitySummary.traversableTileCount > metadata.traversabilitySummary.blockedTileCount)
            {
                supportedChunk = content;
            }
        }
    }

    if (!supportedChunk.has_value() || supportedChunk->instances.empty())
    {
        return false;
    }

    if (supportedChunk->chunkCoordinates.x != world.getChunkCoordinates(world.getTileCoordinates(supportedChunk->instances.front().position)).x
        || supportedChunk->chunkCoordinates.y != world.getChunkCoordinates(world.getTileCoordinates(supportedChunk->instances.front().position)).y)
    {
        return false;
    }

    for (const rpg::ContentInstance& instance : supportedChunk->instances)
    {
        const rpg::ChunkCoordinates owningChunk = world.getChunkCoordinates(world.getTileCoordinates(instance.position));

        if (instance.id == 0
            || owningChunk.x != supportedChunk->chunkCoordinates.x
            || owningChunk.y != supportedChunk->chunkCoordinates.y
            || instance.footprint.size.width <= 0.0F
            || instance.footprint.size.height <= 0.0F
            || instance.appearanceId.value == 0)
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool isWaterVegetationPrototypeId(const std::string& prototypeId) noexcept
{
    return prototypeId.starts_with("water_lily_") || prototypeId.starts_with("marsh_reeds_");
}

[[nodiscard]] const char* getPlacementClassName(const rpg::TileType tileType) noexcept
{
    switch (tileType)
    {
    case rpg::TileType::Grass:
        return "grass";
    case rpg::TileType::Sand:
        return "sand";
    case rpg::TileType::Water:
        return "water";
    case rpg::TileType::Forest:
        return "forest";
    }

    return "";
}

[[nodiscard]] bool prototypeSupportsTileType(
    const rpg::detail::VegetationPrototype& prototype,
    const rpg::TileType tileType) noexcept
{
    const std::string_view placementClass = getPlacementClassName(tileType);
    const auto placeOnIt = std::find(prototype.placeOn.begin(), prototype.placeOn.end(), placementClass);

    if (placeOnIt == prototype.placeOn.end())
    {
        return false;
    }

    const auto biomeIt = std::find_if(
        prototype.biomes.begin(),
        prototype.biomes.end(),
        [placementClass](const auto& entry)
        {
            return entry.first == placementClass;
        });

    return biomeIt != prototype.biomes.end() && biomeIt->second > 0.0F;
}

[[nodiscard]] bool verifyVisibleContentQueries()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    std::optional<rpg::ContentInstance> sampleInstance;

    for (int chunkY = -12; chunkY <= 12 && !sampleInstance.has_value(); ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});

            if (!content.instances.empty())
            {
                sampleInstance = content.instances.front();
                break;
            }
        }
    }

    if (!sampleInstance.has_value())
    {
        return false;
    }

    const float frameSize = world.getTileSize() * 0.5F;
    const float overlap = world.getTileSize() * 0.25F;
    const float footprintLeft = sampleInstance->position.x + sampleInstance->footprint.offset.x;
    const float footprintRight = footprintLeft + sampleInstance->footprint.size.width;
    const float footprintVerticalCenter =
        sampleInstance->position.y + sampleInstance->footprint.offset.y + (sampleInstance->footprint.size.height * 0.5F);
    const rpg::ViewFrame intersectingFrame{
        {
            footprintRight + (frameSize * 0.5F) - overlap,
            footprintVerticalCenter,
        },
        {frameSize, frameSize}};
    const rpg::ViewFrame nonIntersectingFrame{
        {
            footprintRight + frameSize,
            footprintVerticalCenter,
        },
        {frameSize, frameSize}};

    const std::vector<rpg::VisibleWorldContent> intersectingContent = world.getVisibleContent(intersectingFrame);
    const std::vector<rpg::VisibleWorldContent> nonIntersectingContent = world.getVisibleContent(nonIntersectingFrame);

    return containsVisibleContent(intersectingContent, *sampleInstance)
        && !containsVisibleContent(nonIntersectingContent, *sampleInstance);
}

[[nodiscard]] bool verifyForestChunksProduceDenserVegetationThanGrassChunks()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    int forestChunkCount = 0;
    int grassChunkCount = 0;
    int forestTreeCount = 0;
    int grassTreeCount = 0;
    int forestInstanceCount = 0;
    int grassInstanceCount = 0;

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkCoordinates chunkCoordinates{chunkX, chunkY};
            const rpg::ChunkMetadata metadata = world.getChunkMetadata(chunkCoordinates);
            const rpg::ChunkContent content = world.getChunkContent(chunkCoordinates);
            const int treeCount = static_cast<int>(std::count_if(
                content.instances.begin(),
                content.instances.end(),
                [](const rpg::ContentInstance& instance)
                {
                    return instance.type == rpg::ContentType::Tree;
                }));

            if (metadata.biomeSummary.forestTileCount > metadata.biomeSummary.grassTileCount
                && metadata.biomeSummary.forestTileCount > 0)
            {
                ++forestChunkCount;
                forestTreeCount += treeCount;
                forestInstanceCount += static_cast<int>(content.instances.size());
                continue;
            }

            if (metadata.biomeSummary.grassTileCount > metadata.biomeSummary.forestTileCount
                && metadata.biomeSummary.grassTileCount > 0)
            {
                ++grassChunkCount;
                grassTreeCount += treeCount;
                grassInstanceCount += static_cast<int>(content.instances.size());
            }
        }
    }

    return forestChunkCount > 0
        && grassChunkCount > 0
        && forestTreeCount * grassChunkCount > grassTreeCount * forestChunkCount
        && forestInstanceCount * grassChunkCount > grassInstanceCount * forestChunkCount;
}

[[nodiscard]] bool verifyWaterVegetationPlacementIsDeterministic()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World firstWorld(config);
    const rpg::World secondWorld(config);
    std::optional<rpg::ChunkCoordinates> sampleChunk;

    for (int chunkY = -12; chunkY <= 12 && !sampleChunk.has_value(); ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkCoordinates chunkCoordinates{chunkX, chunkY};
            const rpg::ChunkContent content = firstWorld.getChunkContent(chunkCoordinates);
            const auto waterVegetationIt = std::find_if(
                content.instances.begin(),
                content.instances.end(),
                [](const rpg::ContentInstance& instance)
                {
                    return isWaterVegetationPrototypeId(instance.prototypeId);
                });

            if (waterVegetationIt != content.instances.end())
            {
                sampleChunk = chunkCoordinates;
                break;
            }
        }
    }

    if (!sampleChunk.has_value())
    {
        return false;
    }

    const rpg::ChunkContent firstContent = firstWorld.getChunkContent(*sampleChunk);
    const rpg::ChunkContent repeatedContent = firstWorld.getChunkContent(*sampleChunk);
    const rpg::ChunkContent secondContent = secondWorld.getChunkContent(*sampleChunk);

    if (!areEqual(firstContent, repeatedContent) || !areEqual(firstContent, secondContent))
    {
        return false;
    }

    bool foundWaterVegetation = false;

    for (const rpg::ContentInstance& instance : firstContent.instances)
    {
        if (!isWaterVegetationPrototypeId(instance.prototypeId))
        {
            continue;
        }

        foundWaterVegetation = true;

        if (firstWorld.getTileType(instance.anchorTile) != rpg::TileType::Water)
        {
            return false;
        }
    }

    return foundWaterVegetation;
}

[[nodiscard]] bool verifyVegetationPlacementRespectsPrototypeConstraints()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    const rpg::detail::VegetationTilesetMetadata metadata =
        rpg::detail::loadVegetationTilesetMetadata(std::filesystem::path(RPG_DEFAULT_ASSET_ROOT_PATH));
    bool foundWaterVegetation = false;

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});

            for (const rpg::ContentInstance& instance : content.instances)
            {
                const rpg::detail::VegetationPrototype& prototype = metadata.getPrototypeById(instance.prototypeId);
                const rpg::TileType anchorTileType = world.getTileType(instance.anchorTile);

                if (!prototypeSupportsTileType(prototype, anchorTileType))
                {
                    return false;
                }

                if (isWaterVegetationPrototypeId(instance.prototypeId))
                {
                    foundWaterVegetation = true;
                }
            }
        }
    }

    return foundWaterVegetation;
}

[[nodiscard]] bool verifyLargeVegetationRemainsVisibleAcrossChunkBoundaries()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    std::optional<rpg::ContentInstance> spanningInstance;
    constexpr int kChunkSizeInTiles = rpg::detail::getChunkSizeInTiles();

    for (int chunkY = -12; chunkY <= 12 && !spanningInstance.has_value(); ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});
            const float chunkTop = static_cast<float>(chunkY * kChunkSizeInTiles) * world.getTileSize();

            for (const rpg::ContentInstance& instance : content.instances)
            {
                const float top = instance.position.y + instance.footprint.offset.y;

                if (instance.type == rpg::ContentType::Tree && top < chunkTop)
                {
                    spanningInstance = instance;
                    break;
                }
            }

            if (spanningInstance.has_value())
            {
                break;
            }
        }
    }

    if (!spanningInstance.has_value())
    {
        return false;
    }

    const float frameSize = world.getTileSize() * 0.5F;
    const float top = spanningInstance->position.y + spanningInstance->footprint.offset.y;
    const float frameCenterX = spanningInstance->position.x;
    const float frameCenterY = top + (frameSize * 0.5F);
    const std::vector<rpg::VisibleWorldContent> visibleContent = world.getVisibleContent(
        {{frameCenterX, frameCenterY}, {frameSize, frameSize}});

    return containsVisibleContent(visibleContent, *spanningInstance);
}

[[nodiscard]] bool verifyVegetationFootprintsMatchMetadataBounds()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    const rpg::detail::VegetationTilesetMetadata metadata =
        rpg::detail::loadVegetationTilesetMetadata(std::filesystem::path(RPG_DEFAULT_ASSET_ROOT_PATH));

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});

            for (const rpg::ContentInstance& instance : content.instances)
            {
                const rpg::detail::VegetationPrototype& prototype = metadata.getPrototypeById(instance.prototypeId);
                const float expectedOffsetX = (static_cast<float>(prototype.bounds.minOffsetX) - 0.5F) * world.getTileSize();
                const float expectedOffsetY = (static_cast<float>(prototype.bounds.minOffsetY) - 0.5F) * world.getTileSize();
                const float expectedWidth = static_cast<float>(
                    prototype.bounds.maxOffsetX - prototype.bounds.minOffsetX + 1) * world.getTileSize();
                const float expectedHeight = static_cast<float>(
                    prototype.bounds.maxOffsetY - prototype.bounds.minOffsetY + 1) * world.getTileSize();

                if (!areClose(instance.footprint.offset.x, expectedOffsetX)
                    || !areClose(instance.footprint.offset.y, expectedOffsetY)
                    || !areClose(instance.footprint.size.width, expectedWidth)
                    || !areClose(instance.footprint.size.height, expectedHeight))
                {
                    return false;
                }
            }
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

[[nodiscard]] bool verifyUnsupportedTerrainShapesAreRemoved()
{
    const std::array<rpg::WorldConfig, 3> configs = {{
        {.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F},
        {.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F},
        {.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F},
    }};
    const std::array<rpg::TileCoordinates, 3> windowAnchors = {{
        {0, 0},
        {rpg::detail::getChunkSizeInTiles() * 7, -rpg::detail::getChunkSizeInTiles() * 5},
        {-rpg::detail::getChunkSizeInTiles() * 6, rpg::detail::getChunkSizeInTiles() * 4},
    }};
    constexpr int kHalfWindowSizeInTiles = rpg::detail::getChunkSizeInTiles();

    for (const rpg::WorldConfig& config : configs)
    {
        const rpg::World world(config);

        for (const rpg::TileCoordinates& anchor : windowAnchors)
        {
            for (int y = anchor.y - kHalfWindowSizeInTiles; y <= anchor.y + kHalfWindowSizeInTiles; ++y)
            {
                for (int x = anchor.x - kHalfWindowSizeInTiles; x <= anchor.x + kHalfWindowSizeInTiles; ++x)
                {
                    if (hasUnsupportedTerrainShape(world, {x, y}))
                    {
                        return false;
                    }
                }
            }
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

    if (!verifyContentQueriesGenerateAndReuseWorldCache())
    {
        return 1;
    }

    if (!verifyChunkContentRecords())
    {
        return 1;
    }

    if (!verifyVisibleContentQueries())
    {
        return 1;
    }

    if (!verifyForestChunksProduceDenserVegetationThanGrassChunks())
    {
        return 1;
    }

    if (!verifyWaterVegetationPlacementIsDeterministic())
    {
        return 1;
    }

    if (!verifyVegetationPlacementRespectsPrototypeConstraints())
    {
        return 1;
    }

    if (!verifyLargeVegetationRemainsVisibleAcrossChunkBoundaries())
    {
        return 1;
    }

    if (!verifyVegetationFootprintsMatchMetadataBounds())
    {
        return 1;
    }

    if (!verifyAbsoluteCoordinateSignalsAreWorldSizeIndependent())
    {
        return 1;
    }

    if (!verifyUnsupportedTerrainShapesAreRemoved())
    {
        return 1;
    }

    return 0;
}
