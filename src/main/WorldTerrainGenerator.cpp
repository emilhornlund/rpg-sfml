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
    const BiomeSampler biomeSampler{m_config};

    for (int localY = 0; localY < getChunkSizeInTiles(); ++localY)
    {
        for (int localX = 0; localX < getChunkSizeInTiles(); ++localX)
        {
            const TileCoordinates localCoordinates{localX, localY};
            const TileCoordinates worldCoordinates = getWorldTileCoordinates(chunkX, chunkY, localCoordinates);

            chunkData.tiles[toIndex(localCoordinates, getChunkSizeInTiles())] =
                biomeSampler.sampleTileType(worldCoordinates.x, worldCoordinates.y);
        }
    }

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
