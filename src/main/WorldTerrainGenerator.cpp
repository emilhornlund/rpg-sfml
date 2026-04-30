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
#include <cmath>
#include <cstddef>
#include <cstdint>
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

[[nodiscard]] float clamp01(const float value) noexcept
{
    return std::clamp(value, 0.0F, 1.0F);
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

[[nodiscard]] float smoothstep(const float value) noexcept
{
    const float clampedValue = clamp01(value);
    return clampedValue * clampedValue * (3.0F - (2.0F * clampedValue));
}

[[nodiscard]] float interpolate(const float start, const float end, const float factor) noexcept
{
    return start + ((end - start) * factor);
}

[[nodiscard]] float toUnitFloat(const std::uint32_t seed, const int x, const int y) noexcept
{
    return static_cast<float>(hashCoordinates(seed, x, y))
        / static_cast<float>(std::numeric_limits<std::uint32_t>::max());
}

[[nodiscard]] float sampleValueNoise(
    const std::uint32_t seed,
    const float sampleX,
    const float sampleY) noexcept
{
    const int originX = static_cast<int>(std::floor(sampleX));
    const int originY = static_cast<int>(std::floor(sampleY));
    const float fractionX = smoothstep(sampleX - static_cast<float>(originX));
    const float fractionY = smoothstep(sampleY - static_cast<float>(originY));

    const float topLeft = toUnitFloat(seed, originX, originY);
    const float topRight = toUnitFloat(seed, originX + 1, originY);
    const float bottomLeft = toUnitFloat(seed, originX, originY + 1);
    const float bottomRight = toUnitFloat(seed, originX + 1, originY + 1);

    const float top = interpolate(topLeft, topRight, fractionX);
    const float bottom = interpolate(bottomLeft, bottomRight, fractionX);
    return interpolate(top, bottom, fractionY);
}

[[nodiscard]] float toNoiseCoordinate(const int coordinate, const float scaleInTiles) noexcept
{
    return static_cast<float>(coordinate) / scaleInTiles;
}

[[nodiscard]] float evaluateEdgeFalloff(const WorldConfig& config, const int x, const int y) noexcept
{
    constexpr float kEdgeFadeDistanceInTiles = 10.0F;

    const int distanceToVerticalEdge = std::min(x, (config.widthInTiles - 1) - x);
    const int distanceToHorizontalEdge = std::min(y, (config.heightInTiles - 1) - y);
    const int minDistanceToEdge = std::min(distanceToVerticalEdge, distanceToHorizontalEdge);

    return smoothstep(static_cast<float>(minDistanceToEdge) / kEdgeFadeDistanceInTiles);
}

[[nodiscard]] float evaluateElevation(const WorldConfig& config, const int x, const int y) noexcept
{
    const float sampleX = toNoiseCoordinate(x, 24.0F);
    const float sampleY = toNoiseCoordinate(y, 24.0F);
    const float continent = sampleValueNoise(config.seed ^ 0xA511E9B3U, sampleX, sampleY);
    const float region = sampleValueNoise(config.seed ^ 0x63D83595U, sampleX * 2.4F, sampleY * 2.4F);
    const float detail = sampleValueNoise(config.seed ^ 0xC2B2AE35U, sampleX * 4.8F, sampleY * 4.8F);
    const float baseElevation = (0.58F * continent) + (0.27F * region) + (0.15F * detail);
    const float edgeFalloff = evaluateEdgeFalloff(config, x, y);
    return clamp01(baseElevation - ((1.0F - edgeFalloff) * 0.48F));
}

[[nodiscard]] float evaluateMoisture(const WorldConfig& config, const int x, const int y) noexcept
{
    const float climate = sampleValueNoise(
        config.seed ^ 0x9E3779B9U,
        toNoiseCoordinate(x, 18.0F) + 11.0F,
        toNoiseCoordinate(y, 18.0F) + 7.0F);
    const float localVariation = sampleValueNoise(
        config.seed ^ 0x7F4A7C15U,
        toNoiseCoordinate(x, 6.0F) + 19.0F,
        toNoiseCoordinate(y, 6.0F) + 13.0F);
    return clamp01((0.68F * climate) + (0.32F * localVariation));
}

[[nodiscard]] TileType classifyTile(const WorldConfig& config, const int x, const int y) noexcept
{
    if (x < 0
        || y < 0
        || x >= config.widthInTiles
        || y >= config.heightInTiles
        || x == 0
        || y == 0
        || x == config.widthInTiles - 1
        || y == config.heightInTiles - 1)
    {
        return TileType::Water;
    }

    constexpr float kSeaLevel = 0.39F;
    constexpr float kShorelineLevel = 0.47F;
    constexpr float kForestMoisture = 0.53F;

    const float elevation = evaluateElevation(config, x, y);
    const float moisture = evaluateMoisture(config, x, y);

    if (elevation < kSeaLevel)
    {
        return TileType::Water;
    }

    if (elevation < kShorelineLevel)
    {
        return TileType::Sand;
    }

    if (moisture >= kForestMoisture)
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

    if (config.widthInTiles <= 0 || config.heightInTiles <= 0)
    {
        return worldData;
    }

    std::map<std::pair<int, int>, GeneratedChunkData> generatedChunks;
    const int lastChunkX = getChunkCoordinate(config.widthInTiles - 1);
    const int lastChunkY = getChunkCoordinate(config.heightInTiles - 1);

    for (int chunkY = 0; chunkY <= lastChunkY; ++chunkY)
    {
        for (int chunkX = 0; chunkX <= lastChunkX; ++chunkX)
        {
            generatedChunks.emplace(
                std::make_pair(chunkX, chunkY),
                generateChunkData(config, chunkX, chunkY));
        }
    }

    for (int y = 0; y < config.heightInTiles; ++y)
    {
        for (int x = 0; x < config.widthInTiles; ++x)
        {
            const int chunkX = getChunkCoordinate(x);
            const int chunkY = getChunkCoordinate(y);
            const TileCoordinates localCoordinates{
                getChunkLocalCoordinate(x),
                getChunkLocalCoordinate(y)};
            const GeneratedChunkData& chunk = generatedChunks.at(std::make_pair(chunkX, chunkY));

            worldData.tiles[toIndex({x, y}, config.widthInTiles)] =
                chunk.tiles[toIndex(localCoordinates, getChunkSizeInTiles())];
        }
    }

    worldData.spawnTile = findSpawnTile(worldData.tiles, config.widthInTiles, config.heightInTiles);
    return worldData;
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

GeneratedChunkData generateChunkData(const WorldConfig& config, const int chunkX, const int chunkY)
{
    GeneratedChunkData chunkData;
    chunkData.chunkX = chunkX;
    chunkData.chunkY = chunkY;
    chunkData.tiles.resize(static_cast<std::size_t>(getChunkSizeInTiles() * getChunkSizeInTiles()));

    for (int localY = 0; localY < getChunkSizeInTiles(); ++localY)
    {
        for (int localX = 0; localX < getChunkSizeInTiles(); ++localX)
        {
            const TileCoordinates localCoordinates{localX, localY};
            const TileCoordinates worldCoordinates = getWorldTileCoordinates(chunkX, chunkY, localCoordinates);

            chunkData.tiles[toIndex(localCoordinates, getChunkSizeInTiles())] =
                classifyTile(config, worldCoordinates.x, worldCoordinates.y);
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
