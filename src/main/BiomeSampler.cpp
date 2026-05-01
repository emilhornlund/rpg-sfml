/**
 * @file BiomeSampler.cpp
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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace rpg
{
namespace detail
{

namespace
{

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

} // namespace

BiomeSampler::BiomeSampler(const WorldConfig& config) noexcept
    : m_config(config)
{
}

TileType BiomeSampler::sampleTileType(const int x, const int y) const noexcept
{
    constexpr float kSeaLevel = 0.39F;
    constexpr float kShorelineLevel = 0.47F;
    constexpr float kForestMoisture = 0.53F;

    const float elevation = evaluateElevation(x, y);
    const float moisture = evaluateMoisture(x, y);

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

float BiomeSampler::evaluateElevation(const int x, const int y) const noexcept
{
    const float sampleX = toNoiseCoordinate(x, 24.0F);
    const float sampleY = toNoiseCoordinate(y, 24.0F);
    const float continent = sampleValueNoise(m_config.seed ^ 0xA511E9B3U, sampleX, sampleY);
    const float region = sampleValueNoise(m_config.seed ^ 0x63D83595U, sampleX * 2.4F, sampleY * 2.4F);
    const float detail = sampleValueNoise(m_config.seed ^ 0xC2B2AE35U, sampleX * 4.8F, sampleY * 4.8F);
    return clamp01((0.58F * continent) + (0.27F * region) + (0.15F * detail));
}

float BiomeSampler::evaluateMoisture(const int x, const int y) const noexcept
{
    const float climate = sampleValueNoise(
        m_config.seed ^ 0x9E3779B9U,
        toNoiseCoordinate(x, 18.0F) + 11.0F,
        toNoiseCoordinate(y, 18.0F) + 7.0F);
    const float localVariation = sampleValueNoise(
        m_config.seed ^ 0x7F4A7C15U,
        toNoiseCoordinate(x, 6.0F) + 19.0F,
        toNoiseCoordinate(y, 6.0F) + 13.0F);
    return clamp01((0.68F * climate) + (0.32F * localVariation));
}

} // namespace detail
} // namespace rpg
