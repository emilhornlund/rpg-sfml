/**
 * @file RoadOverlayWorldSupport.hpp
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

#ifndef RPG_MAIN_ROAD_OVERLAY_WORLD_SUPPORT_HPP
#define RPG_MAIN_ROAD_OVERLAY_WORLD_SUPPORT_HPP

#include <main/World.hpp>

#include <array>
#include <cstdint>
#include <optional>

namespace rpg
{
namespace detail
{

enum class RoadOverlayAxis
{
    Horizontal,
    Vertical
};

[[nodiscard]] constexpr bool supportsRoadOverlaySurface(const TileType tileType) noexcept
{
    return tileType == TileType::Grass || tileType == TileType::Sand || tileType == TileType::Forest;
}

[[nodiscard]] constexpr int absoluteRoadOffset(const int value) noexcept
{
    return value < 0 ? -value : value;
}

[[nodiscard]] inline std::uint32_t hashRoadOverlayWideningSelection(
    const std::uint32_t seed,
    const int segmentIndex,
    const RoadOverlayAxis axis,
    const bool isPositiveDirection) noexcept
{
    std::uint32_t value = seed ^ (axis == RoadOverlayAxis::Horizontal ? 0x63A79B1DU : 0xB5470917U);
    value ^= static_cast<std::uint32_t>(segmentIndex) * 73856093U;
    value ^= isPositiveDirection ? 0x9E3779B9U : 0x85EBCA6BU;
    value ^= value >> 13U;
    value *= 1274126177U;
    value ^= value >> 16U;
    return value;
}

[[nodiscard]] inline std::optional<int> getRoadWideningLateralOffset(
    const std::uint32_t seed,
    const int axialDelta,
    const RoadOverlayAxis axis) noexcept
{
    constexpr int kRoadHalfLengthInTiles = 48;
    constexpr int kRoadWideningStartDistance = 2;
    constexpr int kRoadWideningEndClearance = 4;
    constexpr int kRoadWideningSegmentLength = 6;
    constexpr int kRoadWideningRunLength = 4;

    const int distance = absoluteRoadOffset(axialDelta);

    if (distance < kRoadWideningStartDistance || distance > kRoadHalfLengthInTiles - kRoadWideningEndClearance)
    {
        return std::nullopt;
    }

    const int distanceFromStart = distance - kRoadWideningStartDistance;
    const int segmentIndex = distanceFromStart / kRoadWideningSegmentLength;
    const int segmentLocalOffset = distanceFromStart % kRoadWideningSegmentLength;

    if (segmentLocalOffset >= kRoadWideningRunLength)
    {
        return std::nullopt;
    }

    const std::uint32_t hash = hashRoadOverlayWideningSelection(seed, segmentIndex, axis, axialDelta >= 0);
    return (hash & 1U) == 0U ? std::optional<int>(-1) : std::optional<int>(2);
}

[[nodiscard]] inline bool hasRawRoadWideningAt(
    const int lateralDelta,
    const int axialDelta,
    const std::uint32_t seed,
    const RoadOverlayAxis axis) noexcept
{
    const std::optional<int> wideningOffset = getRoadWideningLateralOffset(seed, axialDelta, axis);
    return wideningOffset.has_value() && lateralDelta == *wideningOffset;
}

[[nodiscard]] inline bool hasSmoothedRoadWideningAt(
    const int lateralDelta,
    const int axialDelta,
    const std::uint32_t seed,
    const RoadOverlayAxis axis) noexcept
{
    const bool hasCurrentWidening = hasRawRoadWideningAt(lateralDelta, axialDelta, seed, axis);
    const bool hasPreviousWidening = hasRawRoadWideningAt(lateralDelta, axialDelta - 1, seed, axis);
    const bool hasNextWidening = hasRawRoadWideningAt(lateralDelta, axialDelta + 1, seed, axis);

    if (hasCurrentWidening)
    {
        return hasPreviousWidening || hasNextWidening;
    }

    return hasPreviousWidening && hasNextWidening;
}

[[nodiscard]] inline bool occupiesRoadBand(
    const int lateralDelta,
    const int axialDelta,
    const std::uint32_t seed,
    const RoadOverlayAxis axis) noexcept
{
    constexpr int kRoadHalfLengthInTiles = 48;

    if (absoluteRoadOffset(axialDelta) > kRoadHalfLengthInTiles)
    {
        return false;
    }

    if (lateralDelta == 0 || lateralDelta == 1)
    {
        return true;
    }

    return hasSmoothedRoadWideningAt(lateralDelta, axialDelta, seed, axis);
}

[[nodiscard]] inline bool hasRoadOverlayAt(
    const TileCoordinates& coordinates,
    const TileType tileType,
    const TileCoordinates& spawnTile,
    const std::uint32_t seed) noexcept
{
    if (!supportsRoadOverlaySurface(tileType))
    {
        return false;
    }

    const int deltaX = coordinates.x - spawnTile.x;
    const int deltaY = coordinates.y - spawnTile.y;
    return occupiesRoadBand(deltaY, deltaX, seed, RoadOverlayAxis::Horizontal)
        || occupiesRoadBand(deltaX, deltaY, seed, RoadOverlayAxis::Vertical);
}

template <typename TileTypeLookup>
[[nodiscard]] inline std::array<bool, 8> getNeighborRoadOverlayOccupancy(
    const TileCoordinates& coordinates,
    const TileCoordinates& spawnTile,
    const std::uint32_t seed,
    const TileTypeLookup& tileTypeLookup)
{
    const auto hasNeighborRoadOverlay = [&spawnTile, seed, &tileTypeLookup](const int x, const int y)
    {
        const TileCoordinates neighborCoordinates{x, y};
        return hasRoadOverlayAt(
            neighborCoordinates,
            tileTypeLookup(neighborCoordinates),
            spawnTile,
            seed);
    };

    return {
        hasNeighborRoadOverlay(coordinates.x, coordinates.y - 1),
        hasNeighborRoadOverlay(coordinates.x + 1, coordinates.y - 1),
        hasNeighborRoadOverlay(coordinates.x + 1, coordinates.y),
        hasNeighborRoadOverlay(coordinates.x + 1, coordinates.y + 1),
        hasNeighborRoadOverlay(coordinates.x, coordinates.y + 1),
        hasNeighborRoadOverlay(coordinates.x - 1, coordinates.y + 1),
        hasNeighborRoadOverlay(coordinates.x - 1, coordinates.y),
        hasNeighborRoadOverlay(coordinates.x - 1, coordinates.y - 1)};
}

[[nodiscard]] inline bool isClippedSingleTileRoadEndCap(const std::array<bool, 8>& occupancy) noexcept
{
    const bool north = occupancy[0];
    const bool northEast = occupancy[1];
    const bool east = occupancy[2];
    const bool southEast = occupancy[3];
    const bool south = occupancy[4];
    const bool southWest = occupancy[5];
    const bool west = occupancy[6];
    const bool northWest = occupancy[7];
    const int occupiedNeighborCount = static_cast<int>(north) + static_cast<int>(northEast) + static_cast<int>(east)
        + static_cast<int>(southEast) + static_cast<int>(south) + static_cast<int>(southWest)
        + static_cast<int>(west) + static_cast<int>(northWest);

    if (occupiedNeighborCount != 2)
    {
        return false;
    }

    return (north && northEast)
        || (north && northWest)
        || (east && northEast)
        || (east && southEast)
        || (south && southEast)
        || (south && southWest)
        || (west && northWest)
        || (west && southWest);
}

template <typename TileTypeLookup>
[[nodiscard]] inline bool hasPublishedRoadOverlayAt(
    const TileCoordinates& coordinates,
    const TileType tileType,
    const TileCoordinates& spawnTile,
    const std::uint32_t seed,
    const TileTypeLookup& tileTypeLookup)
{
    if (!hasRoadOverlayAt(coordinates, tileType, spawnTile, seed))
    {
        return false;
    }

    return !isClippedSingleTileRoadEndCap(
        getNeighborRoadOverlayOccupancy(
            coordinates,
            spawnTile,
            seed,
            tileTypeLookup));
}

} // namespace detail
} // namespace rpg

#endif // RPG_MAIN_ROAD_OVERLAY_WORLD_SUPPORT_HPP
