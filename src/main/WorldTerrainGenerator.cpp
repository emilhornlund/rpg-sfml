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
#include <limits>
#include <cmath>
#include <cstddef>
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

[[nodiscard]] bool isPointOfInterestTileType(const TileType tileType) noexcept
{
    return tileType == TileType::Sand || tileType == TileType::Forest;
}

[[nodiscard]] int distanceToChunkCenterSquared(const TileCoordinates& localCoordinates) noexcept
{
    const int centerCoordinate = getChunkSizeInTiles() / 2;
    const int deltaX = localCoordinates.x - centerCoordinate;
    const int deltaY = localCoordinates.y - centerCoordinate;
    return (deltaX * deltaX) + (deltaY * deltaY);
}

void appendChunkCandidates(
    ChunkMetadata& metadata,
    const bool hasSpawnCandidate,
    const TileCoordinates& spawnCandidate,
    const bool hasPointOfInterestCandidate,
    const TileCoordinates& pointOfInterestCandidate)
{
    if (hasSpawnCandidate)
    {
        metadata.candidates.push_back({spawnCandidate, ChunkCandidateType::Spawn});
    }

    if (hasPointOfInterestCandidate)
    {
        metadata.candidates.push_back({pointOfInterestCandidate, ChunkCandidateType::PointOfInterest});
    }
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
    GeneratedChunkData chunkData;
    chunkData.chunkX = chunkX;
    chunkData.chunkY = chunkY;
    chunkData.tiles.resize(static_cast<std::size_t>(getChunkSizeInTiles() * getChunkSizeInTiles()));
    chunkData.metadata.chunkCoordinates = {chunkX, chunkY};
    const BiomeSampler biomeSampler{m_config};
    bool hasSpawnCandidate = false;
    TileCoordinates spawnCandidate{0, 0};
    int spawnCandidateDistanceSquared = std::numeric_limits<int>::max();
    bool hasPointOfInterestCandidate = false;
    TileCoordinates pointOfInterestCandidate{0, 0};
    int pointOfInterestCandidateDistanceSquared = std::numeric_limits<int>::max();

    for (int localY = 0; localY < getChunkSizeInTiles(); ++localY)
    {
        for (int localX = 0; localX < getChunkSizeInTiles(); ++localX)
        {
            const TileCoordinates localCoordinates{localX, localY};
            const TileCoordinates worldCoordinates = getWorldTileCoordinates(chunkX, chunkY, localCoordinates);
            const TileType tileType = biomeSampler.sampleTileType(worldCoordinates.x, worldCoordinates.y);
            chunkData.tiles[toIndex(localCoordinates, getChunkSizeInTiles())] = tileType;
            incrementBiomeTileCount(chunkData.metadata.biomeSummary, tileType);

            if (!isTraversableTileType(tileType))
            {
                ++chunkData.metadata.traversabilitySummary.blockedTileCount;
                continue;
            }

            ++chunkData.metadata.traversabilitySummary.traversableTileCount;

            const int centerDistanceSquared = distanceToChunkCenterSquared(localCoordinates);

            if (centerDistanceSquared < spawnCandidateDistanceSquared)
            {
                hasSpawnCandidate = true;
                spawnCandidate = worldCoordinates;
                spawnCandidateDistanceSquared = centerDistanceSquared;
            }

            if (isPointOfInterestTileType(tileType)
                && centerDistanceSquared < pointOfInterestCandidateDistanceSquared)
            {
                hasPointOfInterestCandidate = true;
                pointOfInterestCandidate = worldCoordinates;
                pointOfInterestCandidateDistanceSquared = centerDistanceSquared;
            }
        }
    }

    chunkData.metadata.biomeSummary.dominantTileType = determineDominantTileType(chunkData.metadata.biomeSummary);
    appendChunkCandidates(
        chunkData.metadata,
        hasSpawnCandidate,
        spawnCandidate,
        hasPointOfInterestCandidate,
        pointOfInterestCandidate);

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
