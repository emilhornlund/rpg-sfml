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

#include "BiomeSampler.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <utility>
#include <vector>

namespace rpg
{
namespace detail
{

namespace
{

std::size_t g_generatedChunkCount = 0;
constexpr int kTerrainCleanupPaddingInTiles = getChunkSizeInTiles();
constexpr std::array<TileCoordinates, 4> kCardinalNeighborOffsets = {{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};
constexpr std::array<TileCoordinates, 8> kAllNeighborOffsets = {{{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}}};

[[nodiscard]] std::size_t toIndex(const TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

[[nodiscard]] int floorDivide(const int value, const int divisor) noexcept
{
    const int quotient = value / divisor;
    const int remainder = value % divisor;
    return remainder < 0 ? quotient - 1 : quotient;
}

[[nodiscard]] int floorModulo(const int value, const int divisor) noexcept
{
    const int remainder = value % divisor;
    return remainder < 0 ? remainder + divisor : remainder;
}

[[nodiscard]] TileType getGeneratedTileType(
    const TerrainGenerator& terrainGenerator,
    std::map<std::pair<int, int>, GeneratedChunkData>& generatedChunks,
    const TileCoordinates& coordinates)
{
    const int chunkX = getChunkCoordinate(coordinates.x);
    const int chunkY = getChunkCoordinate(coordinates.y);
    const auto chunkKey = std::make_pair(chunkX, chunkY);
    auto chunkIt = generatedChunks.find(chunkKey);

    if (chunkIt == generatedChunks.end())
    {
        chunkIt = generatedChunks.emplace(chunkKey, terrainGenerator.generateChunk(chunkX, chunkY)).first;
    }

    const TileCoordinates localCoordinates{
        getChunkLocalCoordinate(coordinates.x),
        getChunkLocalCoordinate(coordinates.y)};
    return chunkIt->second.tiles[toIndex(localCoordinates, getChunkSizeInTiles())];
}

void incrementBiomeTileCount(ChunkBiomeSummary& summary, const TileType tileType) noexcept
{
    switch (tileType)
    {
    case TileType::Water:
        ++summary.waterTileCount;
        break;
    case TileType::Sand:
        ++summary.sandTileCount;
        break;
    case TileType::Grass:
        ++summary.grassTileCount;
        break;
    case TileType::Forest:
        ++summary.forestTileCount;
        break;
    }
}

[[nodiscard]] TileType determineDominantTileType(const ChunkBiomeSummary& summary) noexcept
{
    TileType dominantTileType = TileType::Water;
    int dominantCount = summary.waterTileCount;

    if (summary.sandTileCount > dominantCount)
    {
        dominantTileType = TileType::Sand;
        dominantCount = summary.sandTileCount;
    }

    if (summary.grassTileCount > dominantCount)
    {
        dominantTileType = TileType::Grass;
        dominantCount = summary.grassTileCount;
    }

    if (summary.forestTileCount > dominantCount)
    {
        dominantTileType = TileType::Forest;
    }

    return dominantTileType;
}

[[nodiscard]] int getTerrainCleanupPriority(const TileType tileType) noexcept
{
    switch (tileType)
    {
    case TileType::Water:
        return 4;
    case TileType::Sand:
        return 3;
    case TileType::Forest:
        return 2;
    case TileType::Grass:
        return 1;
    }

    return 0;
}

struct CardinalNeighborMask
{
    bool north = false;
    bool east = false;
    bool south = false;
    bool west = false;
};

[[nodiscard]] CardinalNeighborMask getMatchingCardinalNeighborMask(
    const std::vector<TileType>& tiles,
    const TileCoordinates& coordinates,
    const int widthInTiles,
    const TileType tileType) noexcept
{
    return {
        tiles[toIndex({coordinates.x, coordinates.y - 1}, widthInTiles)] == tileType,
        tiles[toIndex({coordinates.x + 1, coordinates.y}, widthInTiles)] == tileType,
        tiles[toIndex({coordinates.x, coordinates.y + 1}, widthInTiles)] == tileType,
        tiles[toIndex({coordinates.x - 1, coordinates.y}, widthInTiles)] == tileType,
    };
}

[[nodiscard]] bool isUnsupportedTerrainShape(
    const std::vector<TileType>& tiles,
    const TileCoordinates& coordinates,
    const int widthInTiles,
    const TileType tileType) noexcept
{
    const CardinalNeighborMask mask = getMatchingCardinalNeighborMask(tiles, coordinates, widthInTiles, tileType);
    const int cardinalNeighborCount =
        static_cast<int>(mask.north) + static_cast<int>(mask.east) + static_cast<int>(mask.south) + static_cast<int>(mask.west);

    return cardinalNeighborCount <= 1;
}

[[nodiscard]] TileType selectReplacementTileType(
    const std::vector<TileType>& tiles,
    const TileCoordinates& coordinates,
    const int widthInTiles,
    const TileType currentTileType) noexcept
{
    int waterCount = 0;
    int sandCount = 0;
    int grassCount = 0;
    int forestCount = 0;

    for (const TileCoordinates& offset : kAllNeighborOffsets)
    {
        const TileType neighborTileType = tiles[toIndex({coordinates.x + offset.x, coordinates.y + offset.y}, widthInTiles)];

        if (neighborTileType == currentTileType)
        {
            continue;
        }

        switch (neighborTileType)
        {
        case TileType::Water:
            ++waterCount;
            break;
        case TileType::Sand:
            ++sandCount;
            break;
        case TileType::Grass:
            ++grassCount;
            break;
        case TileType::Forest:
            ++forestCount;
            break;
        }
    }

    TileType selectedTileType = currentTileType;
    int bestCount = -1;
    int bestPriority = std::numeric_limits<int>::min();

    for (const auto& [candidateTileType, candidateCount] : std::array<std::pair<TileType, int>, 4>{{
             {TileType::Water, waterCount},
             {TileType::Sand, sandCount},
             {TileType::Grass, grassCount},
             {TileType::Forest, forestCount},
         }})
    {
        const int candidatePriority = getTerrainCleanupPriority(candidateTileType);

        if (candidateCount > bestCount || (candidateCount == bestCount && candidatePriority > bestPriority))
        {
            bestCount = candidateCount;
            bestPriority = candidatePriority;
            selectedTileType = candidateTileType;
        }
    }

    return selectedTileType;
}

[[nodiscard]] TileType selectReplacementTileTypeExcludingRemoved(
    const std::vector<TileType>& tiles,
    const std::vector<bool>& removedMask,
    const TileCoordinates& coordinates,
    const int widthInTiles,
    const TileType currentTileType) noexcept
{
    int waterCount = 0;
    int sandCount = 0;
    int grassCount = 0;
    int forestCount = 0;

    for (const TileCoordinates& offset : kAllNeighborOffsets)
    {
        const TileCoordinates neighbor{coordinates.x + offset.x, coordinates.y + offset.y};
        const std::size_t neighborIndex = toIndex(neighbor, widthInTiles);

        if (removedMask[neighborIndex])
        {
            continue;
        }

        const TileType neighborTileType = tiles[neighborIndex];

        if (neighborTileType == currentTileType)
        {
            continue;
        }

        switch (neighborTileType)
        {
        case TileType::Water:
            ++waterCount;
            break;
        case TileType::Sand:
            ++sandCount;
            break;
        case TileType::Grass:
            ++grassCount;
            break;
        case TileType::Forest:
            ++forestCount;
            break;
        }
    }

    if (waterCount == 0 && sandCount == 0 && grassCount == 0 && forestCount == 0)
    {
        return selectReplacementTileType(tiles, coordinates, widthInTiles, currentTileType);
    }

    TileType selectedTileType = currentTileType;
    int bestCount = -1;
    int bestPriority = std::numeric_limits<int>::min();

    for (const auto& [candidateTileType, candidateCount] : std::array<std::pair<TileType, int>, 4>{{
             {TileType::Water, waterCount},
             {TileType::Sand, sandCount},
             {TileType::Grass, grassCount},
             {TileType::Forest, forestCount},
         }})
    {
        const int candidatePriority = getTerrainCleanupPriority(candidateTileType);

        if (candidateCount > bestCount || (candidateCount == bestCount && candidatePriority > bestPriority))
        {
            bestCount = candidateCount;
            bestPriority = candidatePriority;
            selectedTileType = candidateTileType;
        }
    }

    return selectedTileType;
}

[[nodiscard]] bool applyUnsupportedShapeCleanupPass(
    std::vector<TileType>& tiles,
    const int widthInTiles,
    const int heightInTiles)
{
    std::vector<TileType> nextTiles = tiles;
    bool changed = false;

    for (int y = 1; y < heightInTiles - 1; ++y)
    {
        for (int x = 1; x < widthInTiles - 1; ++x)
        {
            const TileCoordinates coordinates{x, y};
            const std::size_t index = toIndex(coordinates, widthInTiles);
            const TileType tileType = tiles[index];

            if (!isUnsupportedTerrainShape(tiles, coordinates, widthInTiles, tileType))
            {
                continue;
            }

            const TileType replacementTileType = selectReplacementTileType(tiles, coordinates, widthInTiles, tileType);

            if (replacementTileType == tileType)
            {
                continue;
            }

            nextTiles[index] = replacementTileType;
            changed = true;
        }
    }

    if (changed)
    {
        tiles = std::move(nextTiles);
    }

    return changed;
}

void cleanupUnsupportedChunkOutput(
    std::vector<TileType>& tiles,
    const int widthInTiles,
    const int chunkRegionOffsetX,
    const int chunkRegionOffsetY)
{
    constexpr int kMaxChunkCleanupPasses = getChunkSizeInTiles();
    constexpr int kChunkCleanupHaloInTiles = 2;

    for (int pass = 0; pass < kMaxChunkCleanupPasses; ++pass)
    {
        std::vector<bool> removedMask(tiles.size(), false);
        bool hasUnsupportedTile = false;

        for (int localY = -kChunkCleanupHaloInTiles; localY < getChunkSizeInTiles() + kChunkCleanupHaloInTiles; ++localY)
        {
            for (int localX = -kChunkCleanupHaloInTiles; localX < getChunkSizeInTiles() + kChunkCleanupHaloInTiles; ++localX)
            {
                const TileCoordinates regionCoordinates{
                    chunkRegionOffsetX + localX + kTerrainCleanupPaddingInTiles,
                    chunkRegionOffsetY + localY + kTerrainCleanupPaddingInTiles};
                const std::size_t index = toIndex(regionCoordinates, widthInTiles);
                const TileType tileType = tiles[index];

                if (!isUnsupportedTerrainShape(tiles, regionCoordinates, widthInTiles, tileType))
                {
                    continue;
                }

                removedMask[index] = true;
                hasUnsupportedTile = true;
            }
        }

        if (!hasUnsupportedTile)
        {
            break;
        }

        std::vector<TileType> nextTiles = tiles;
        bool changed = false;

        for (int localY = -kChunkCleanupHaloInTiles; localY < getChunkSizeInTiles() + kChunkCleanupHaloInTiles; ++localY)
        {
            for (int localX = -kChunkCleanupHaloInTiles; localX < getChunkSizeInTiles() + kChunkCleanupHaloInTiles; ++localX)
            {
                const TileCoordinates regionCoordinates{
                    chunkRegionOffsetX + localX + kTerrainCleanupPaddingInTiles,
                    chunkRegionOffsetY + localY + kTerrainCleanupPaddingInTiles};
                const std::size_t index = toIndex(regionCoordinates, widthInTiles);

                if (!removedMask[index])
                {
                    continue;
                }

                const TileType tileType = tiles[index];
                const TileType replacementTileType =
                    selectReplacementTileTypeExcludingRemoved(tiles, removedMask, regionCoordinates, widthInTiles, tileType);

                if (replacementTileType == tileType)
                {
                    continue;
                }

                nextTiles[index] = replacementTileType;
                changed = true;
            }
        }

        if (!changed)
        {
            break;
        }

        tiles = std::move(nextTiles);
    }
}

void cleanupUnsupportedTerrainShapes(
    std::vector<TileType>& tiles,
    const int widthInTiles,
    const int heightInTiles)
{
    const int maxPasses = widthInTiles + heightInTiles;

    for (int pass = 0; pass < maxPasses; ++pass)
    {
        if (!applyUnsupportedShapeCleanupPass(tiles, widthInTiles, heightInTiles))
        {
            break;
        }
    }
}

[[nodiscard]] std::vector<TileType> generateCleanedChunkRegion(
    const BiomeSampler& biomeSampler,
    const int minChunkX,
    const int maxChunkX,
    const int minChunkY,
    const int maxChunkY)
{
    const int chunkCountX = maxChunkX - minChunkX + 1;
    const int chunkCountY = maxChunkY - minChunkY + 1;
    const int regionWidthInTiles = (chunkCountX * getChunkSizeInTiles()) + (kTerrainCleanupPaddingInTiles * 2);
    const int regionHeightInTiles = (chunkCountY * getChunkSizeInTiles()) + (kTerrainCleanupPaddingInTiles * 2);
    std::vector<TileType> tiles(static_cast<std::size_t>(regionWidthInTiles * regionHeightInTiles));
    const int startWorldX = (minChunkX * getChunkSizeInTiles()) - kTerrainCleanupPaddingInTiles;
    const int startWorldY = (minChunkY * getChunkSizeInTiles()) - kTerrainCleanupPaddingInTiles;

    for (int localY = 0; localY < regionHeightInTiles; ++localY)
    {
        for (int localX = 0; localX < regionWidthInTiles; ++localX)
        {
            tiles[toIndex({localX, localY}, regionWidthInTiles)] =
                biomeSampler.sampleTileType(startWorldX + localX, startWorldY + localY);
        }
    }

    cleanupUnsupportedTerrainShapes(tiles, regionWidthInTiles, regionHeightInTiles);
    return tiles;
}

[[nodiscard]] GeneratedChunkData buildGeneratedChunkData(
    const int chunkX,
    const int chunkY,
    const int minChunkX,
    const int minChunkY,
    const std::vector<TileType>& cleanedChunkRegion,
    const int cleanedRegionWidthInTiles)
{
    GeneratedChunkData chunkData;
    chunkData.chunkX = chunkX;
    chunkData.chunkY = chunkY;
    chunkData.tiles.resize(static_cast<std::size_t>(getChunkSizeInTiles() * getChunkSizeInTiles()));
    chunkData.metadata.chunkCoordinates = {chunkX, chunkY};
    const int chunkRegionOffsetX = (chunkX - minChunkX) * getChunkSizeInTiles();
    const int chunkRegionOffsetY = (chunkY - minChunkY) * getChunkSizeInTiles();

    for (int localY = 0; localY < getChunkSizeInTiles(); ++localY)
    {
        for (int localX = 0; localX < getChunkSizeInTiles(); ++localX)
        {
            const TileCoordinates localCoordinates{localX, localY};
            const TileType tileType = cleanedChunkRegion[toIndex(
                {
                    chunkRegionOffsetX + localCoordinates.x + kTerrainCleanupPaddingInTiles,
                    chunkRegionOffsetY + localCoordinates.y + kTerrainCleanupPaddingInTiles,
                },
                cleanedRegionWidthInTiles)];
            chunkData.tiles[toIndex(localCoordinates, getChunkSizeInTiles())] = tileType;
            incrementBiomeTileCount(chunkData.metadata.biomeSummary, tileType);

            if (!isTraversableTileType(tileType))
            {
                ++chunkData.metadata.traversabilitySummary.blockedTileCount;
                continue;
            }

            ++chunkData.metadata.traversabilitySummary.traversableTileCount;
        }
    }

    chunkData.metadata.biomeSummary.dominantTileType = determineDominantTileType(chunkData.metadata.biomeSummary);
    return chunkData;
}

[[nodiscard]] TileCoordinates findSpawnTile(
    const TerrainGenerator& terrainGenerator,
    std::map<std::pair<int, int>, GeneratedChunkData>& generatedChunks)
{
    constexpr int kSpawnSearchLimitInTiles = getChunkSizeInTiles() * 8;
    const TileCoordinates anchor{0, 0};

    for (int radius = 0; radius <= kSpawnSearchLimitInTiles; ++radius)
    {
        for (int y = anchor.y - radius; y <= anchor.y + radius; ++y)
        {
            for (int x = anchor.x - radius; x <= anchor.x + radius; ++x)
            {
                if (radius > 0
                    && x != anchor.x - radius
                    && x != anchor.x + radius
                    && y != anchor.y - radius
                    && y != anchor.y + radius)
                {
                    continue;
                }

                const TileCoordinates coordinates{x, y};

                if (isTraversableTileType(getGeneratedTileType(terrainGenerator, generatedChunks, coordinates)))
                {
                    return coordinates;
                }
            }
        }
    }

    return anchor;
}

} // namespace

TerrainGenerator::TerrainGenerator(const WorldConfig& config) noexcept
    : m_config(config)
{
}

GeneratedWorldData TerrainGenerator::generateWorld() const
{
    GeneratedWorldData worldData;
    worldData.tiles.resize(static_cast<std::size_t>(m_config.widthInTiles * m_config.heightInTiles));

    if (m_config.widthInTiles <= 0 || m_config.heightInTiles <= 0)
    {
        return worldData;
    }

    std::map<std::pair<int, int>, GeneratedChunkData> generatedChunks;

    for (int y = 0; y < m_config.heightInTiles; ++y)
    {
        for (int x = 0; x < m_config.widthInTiles; ++x)
        {
            worldData.tiles[toIndex({x, y}, m_config.widthInTiles)] =
                getGeneratedTileType(*this, generatedChunks, {x, y});
        }
    }

    worldData.spawnTile = findSpawnTile(*this, generatedChunks);
    return worldData;
}

TileCoordinates TerrainGenerator::generateSpawnTile() const
{
    std::map<std::pair<int, int>, GeneratedChunkData> generatedChunks;
    return findSpawnTile(*this, generatedChunks);
}

int getChunkCoordinate(const int tileCoordinate) noexcept
{
    return floorDivide(tileCoordinate, getChunkSizeInTiles());
}

int getChunkLocalCoordinate(const int tileCoordinate) noexcept
{
    return floorModulo(tileCoordinate, getChunkSizeInTiles());
}

TileCoordinates getWorldTileCoordinates(
    const int chunkX,
    const int chunkY,
    const TileCoordinates& localCoordinates) noexcept
{
    return {
        (chunkX * getChunkSizeInTiles()) + localCoordinates.x,
        (chunkY * getChunkSizeInTiles()) + localCoordinates.y};
}

GeneratedChunkData TerrainGenerator::generateChunk(const int chunkX, const int chunkY) const
{
    const BiomeSampler biomeSampler{m_config};
    std::vector<TileType> cleanedChunkRegion = generateCleanedChunkRegion(biomeSampler, chunkX, chunkX, chunkY, chunkY);
    cleanupUnsupportedChunkOutput(cleanedChunkRegion, getChunkSizeInTiles() + (kTerrainCleanupPaddingInTiles * 2), 0, 0);
    GeneratedChunkData chunkData =
        buildGeneratedChunkData(chunkX, chunkY, chunkX, chunkY, cleanedChunkRegion, getChunkSizeInTiles() + (kTerrainCleanupPaddingInTiles * 2));
    ++g_generatedChunkCount;
    return chunkData;
}

std::size_t getGeneratedChunkCount() noexcept
{
    return g_generatedChunkCount;
}

void resetGeneratedChunkCount() noexcept
{
    g_generatedChunkCount = 0;
}

bool isTraversableTileType(const TileType tileType) noexcept
{
    return tileType != TileType::Water;
}

} // namespace detail
} // namespace rpg
