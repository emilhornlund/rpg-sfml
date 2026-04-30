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
#include <cstdint>
#include <limits>
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

[[nodiscard]] float normalizeCoordinate(const int coordinate, const int maxCoordinate) noexcept
{
    if (maxCoordinate <= 1)
    {
        return 0.0F;
    }

    return static_cast<float>(coordinate) / static_cast<float>(maxCoordinate - 1);
}

[[nodiscard]] float evaluateEdgeFalloff(const float normalizedX, const float normalizedY) noexcept
{
    const float distanceToVerticalEdge = std::min(normalizedX, 1.0F - normalizedX);
    const float distanceToHorizontalEdge = std::min(normalizedY, 1.0F - normalizedY);
    const float minDistanceToEdge = std::min(distanceToVerticalEdge, distanceToHorizontalEdge);
    return smoothstep(minDistanceToEdge / 0.22F);
}

[[nodiscard]] float evaluateElevation(const WorldConfig& config, const int x, const int y) noexcept
{
    const float normalizedX = normalizeCoordinate(x, config.widthInTiles);
    const float normalizedY = normalizeCoordinate(y, config.heightInTiles);
    const float continent = sampleValueNoise(config.seed ^ 0xA511E9B3U, normalizedX * 2.4F, normalizedY * 2.4F);
    const float region = sampleValueNoise(config.seed ^ 0x63D83595U, normalizedX * 5.8F, normalizedY * 5.8F);
    const float detail = sampleValueNoise(config.seed ^ 0xC2B2AE35U, normalizedX * 11.6F, normalizedY * 11.6F);
    const float baseElevation = (0.58F * continent) + (0.27F * region) + (0.15F * detail);
    const float edgeFalloff = evaluateEdgeFalloff(normalizedX, normalizedY);
    return clamp01(baseElevation - ((1.0F - edgeFalloff) * 0.48F));
}

[[nodiscard]] float evaluateMoisture(const WorldConfig& config, const int x, const int y) noexcept
{
    const float normalizedX = normalizeCoordinate(x, config.widthInTiles);
    const float normalizedY = normalizeCoordinate(y, config.heightInTiles);
    const float climate = sampleValueNoise(
        config.seed ^ 0x9E3779B9U,
        (normalizedX * 3.1F) + 11.0F,
        (normalizedY * 3.1F) + 7.0F);
    const float localVariation = sampleValueNoise(
        config.seed ^ 0x7F4A7C15U,
        (normalizedX * 9.4F) + 19.0F,
        (normalizedY * 9.4F) + 13.0F);
    return clamp01((0.68F * climate) + (0.32F * localVariation));
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

    constexpr float kSeaLevel = 0.39F;
    constexpr float kShorelineLevel = 0.47F;
    constexpr float kForestMoisture = 0.59F;

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
