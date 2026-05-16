/**
 * @file RoadNetworkSupport.hpp
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

#ifndef RPG_MAIN_ROAD_NETWORK_SUPPORT_HPP
#define RPG_MAIN_ROAD_NETWORK_SUPPORT_HPP

#include <main/World.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace rpg
{
namespace detail
{

enum class RoadNodeKind
{
    Spawn,
    Junction,
    Destination
};

struct RoadNode
{
    TileCoordinates coordinates{0, 0};
    RoadNodeKind kind = RoadNodeKind::Spawn;
};

struct RoadSegment
{
    std::size_t fromNodeIndex = 0U;
    std::size_t toNodeIndex = 0U;
    std::vector<TileCoordinates> polyline;
};

struct RoadNetwork
{
    TileCoordinates spawnTile{0, 0};
    std::uint32_t seed = 0U;
    std::vector<RoadNode> nodes;
    std::vector<RoadSegment> segments;
};

[[nodiscard]] constexpr bool supportsRoadOverlaySurface(const TileType tileType) noexcept
{
    return tileType == TileType::Grass || tileType == TileType::Sand || tileType == TileType::Forest;
}

[[nodiscard]] inline std::uint32_t hashRoadNetworkSelection(
    const std::uint32_t seed,
    const std::uint32_t salt) noexcept
{
    std::uint32_t value = seed ^ salt;
    value ^= value >> 13U;
    value *= 1274126177U;
    value ^= value >> 16U;
    return value;
}

[[nodiscard]] inline int selectRoadNetworkOffset(
    const std::uint32_t seed,
    const std::uint32_t salt,
    const int minimum,
    const int step,
    const int count) noexcept
{
    if (count <= 0)
    {
        return minimum;
    }

    return minimum + (static_cast<int>(hashRoadNetworkSelection(seed, salt) % static_cast<std::uint32_t>(count)) * step);
}

[[nodiscard]] inline std::size_t appendRoadNode(
    RoadNetwork& network,
    const TileCoordinates& coordinates,
    const RoadNodeKind kind)
{
    network.nodes.push_back({coordinates, kind});
    return network.nodes.size() - 1U;
}

inline void appendRoadSegment(
    RoadNetwork& network,
    const std::size_t fromNodeIndex,
    const std::size_t toNodeIndex,
    const std::vector<TileCoordinates>& polyline)
{
    network.segments.push_back({fromNodeIndex, toNodeIndex, polyline});
}

[[nodiscard]] inline RoadNetwork buildRoadNetwork(
    const TileCoordinates& spawnTile,
    const std::uint32_t seed)
{
    RoadNetwork network;
    network.spawnTile = spawnTile;
    network.seed = seed;

    const int eastJunctionOffset = selectRoadNetworkOffset(seed, 0x6A09E667U, 8, 2, 3);
    const int southJunctionOffset = selectRoadNetworkOffset(seed, 0xBB67AE85U, 8, 2, 3);
    const int eastDestinationOffset = selectRoadNetworkOffset(seed, 0x3C6EF372U, 6, 2, 4);
    const int southDestinationOffset = selectRoadNetworkOffset(seed, 0xA54FF53AU, 6, 2, 4);
    const int northDestinationOffset = selectRoadNetworkOffset(seed, 0x510E527FU, 6, 2, 4);
    const bool hasLoop = (hashRoadNetworkSelection(seed, 0x5BE0CD19U) & 1U) == 0U;

    const std::size_t spawnNodeIndex = appendRoadNode(network, spawnTile, RoadNodeKind::Spawn);
    const std::size_t eastJunctionIndex = appendRoadNode(
        network,
        {spawnTile.x + eastJunctionOffset, spawnTile.y},
        RoadNodeKind::Junction);
    const std::size_t southJunctionIndex = appendRoadNode(
        network,
        {spawnTile.x, spawnTile.y + southJunctionOffset},
        RoadNodeKind::Junction);
    const std::size_t eastDestinationIndex = appendRoadNode(
        network,
        {network.nodes[eastJunctionIndex].coordinates.x + eastDestinationOffset, network.nodes[eastJunctionIndex].coordinates.y},
        RoadNodeKind::Destination);
    const std::size_t southDestinationIndex = appendRoadNode(
        network,
        {network.nodes[southJunctionIndex].coordinates.x, network.nodes[southJunctionIndex].coordinates.y + southDestinationOffset},
        RoadNodeKind::Destination);
    const std::size_t northDestinationIndex = appendRoadNode(
        network,
        {network.nodes[eastJunctionIndex].coordinates.x, network.nodes[eastJunctionIndex].coordinates.y - northDestinationOffset},
        RoadNodeKind::Destination);

    appendRoadSegment(
        network,
        spawnNodeIndex,
        eastJunctionIndex,
        {network.nodes[spawnNodeIndex].coordinates, network.nodes[eastJunctionIndex].coordinates});
    appendRoadSegment(
        network,
        spawnNodeIndex,
        southJunctionIndex,
        {network.nodes[spawnNodeIndex].coordinates, network.nodes[southJunctionIndex].coordinates});
    appendRoadSegment(
        network,
        eastJunctionIndex,
        eastDestinationIndex,
        {network.nodes[eastJunctionIndex].coordinates, network.nodes[eastDestinationIndex].coordinates});
    appendRoadSegment(
        network,
        southJunctionIndex,
        southDestinationIndex,
        {network.nodes[southJunctionIndex].coordinates, network.nodes[southDestinationIndex].coordinates});
    appendRoadSegment(
        network,
        eastJunctionIndex,
        northDestinationIndex,
        {network.nodes[eastJunctionIndex].coordinates, network.nodes[northDestinationIndex].coordinates});

    if (hasLoop)
    {
        appendRoadSegment(
            network,
            eastJunctionIndex,
            southJunctionIndex,
            {
                network.nodes[eastJunctionIndex].coordinates,
                {network.nodes[eastJunctionIndex].coordinates.x, network.nodes[southJunctionIndex].coordinates.y},
                network.nodes[southJunctionIndex].coordinates,
            });
    }

    return network;
}

[[nodiscard]] inline bool occupiesRoadNodeFootprint(
    const TileCoordinates& coordinates,
    const RoadNode& node) noexcept
{
    return coordinates.x >= node.coordinates.x
        && coordinates.x <= node.coordinates.x + 1
        && coordinates.y >= node.coordinates.y
        && coordinates.y <= node.coordinates.y + 1;
}

[[nodiscard]] inline bool occupiesHorizontalRoadRun(
    const TileCoordinates& coordinates,
    const TileCoordinates& start,
    const TileCoordinates& end) noexcept
{
    if (start.y != end.y)
    {
        return false;
    }

    const int minX = start.x < end.x ? start.x : end.x;
    const int maxX = start.x > end.x ? start.x : end.x;
    return coordinates.x >= minX
        && coordinates.x <= maxX
        && (coordinates.y == start.y || coordinates.y == start.y + 1);
}

[[nodiscard]] inline bool occupiesVerticalRoadRun(
    const TileCoordinates& coordinates,
    const TileCoordinates& start,
    const TileCoordinates& end) noexcept
{
    if (start.x != end.x)
    {
        return false;
    }

    const int minY = start.y < end.y ? start.y : end.y;
    const int maxY = start.y > end.y ? start.y : end.y;
    return (coordinates.x == start.x || coordinates.x == start.x + 1)
        && coordinates.y >= minY
        && coordinates.y <= maxY;
}

[[nodiscard]] inline bool occupiesRoadSegmentRun(
    const TileCoordinates& coordinates,
    const TileCoordinates& start,
    const TileCoordinates& end) noexcept
{
    return occupiesHorizontalRoadRun(coordinates, start, end) || occupiesVerticalRoadRun(coordinates, start, end);
}

[[nodiscard]] inline bool occupiesRoadSegment(
    const TileCoordinates& coordinates,
    const RoadSegment& segment) noexcept
{
    if (segment.polyline.size() < 2U)
    {
        return false;
    }

    for (std::size_t pointIndex = 1U; pointIndex < segment.polyline.size(); ++pointIndex)
    {
        if (occupiesRoadSegmentRun(coordinates, segment.polyline[pointIndex - 1U], segment.polyline[pointIndex]))
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] inline bool occupiesRoadNetworkTile(
    const TileCoordinates& coordinates,
    const RoadNetwork& network) noexcept
{
    for (const RoadNode& node : network.nodes)
    {
        if (occupiesRoadNodeFootprint(coordinates, node))
        {
            return true;
        }
    }

    for (const RoadSegment& segment : network.segments)
    {
        if (occupiesRoadSegment(coordinates, segment))
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] inline bool hasRoadOverlayAt(
    const TileCoordinates& coordinates,
    const TileType tileType,
    const RoadNetwork& network) noexcept
{
    return supportsRoadOverlaySurface(tileType) && occupiesRoadNetworkTile(coordinates, network);
}

[[nodiscard]] inline bool hasRoadOverlayAt(
    const TileCoordinates& coordinates,
    const TileType tileType,
    const TileCoordinates& spawnTile,
    const std::uint32_t seed) noexcept
{
    return hasRoadOverlayAt(coordinates, tileType, buildRoadNetwork(spawnTile, seed));
}

template <typename TileTypeLookup>
[[nodiscard]] inline std::array<bool, 8> getNeighborRoadOverlayOccupancy(
    const TileCoordinates& coordinates,
    const RoadNetwork& network,
    const TileTypeLookup& tileTypeLookup)
{
    const auto hasNeighborRoadOverlay = [&network, &tileTypeLookup](const int x, const int y)
    {
        const TileCoordinates neighborCoordinates{x, y};
        return hasRoadOverlayAt(
            neighborCoordinates,
            tileTypeLookup(neighborCoordinates),
            network);
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

template <typename TileTypeLookup>
[[nodiscard]] inline std::array<bool, 8> getNeighborRoadOverlayOccupancy(
    const TileCoordinates& coordinates,
    const TileCoordinates& spawnTile,
    const std::uint32_t seed,
    const TileTypeLookup& tileTypeLookup)
{
    return getNeighborRoadOverlayOccupancy(
        coordinates,
        buildRoadNetwork(spawnTile, seed),
        tileTypeLookup);
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
    const RoadNetwork& network,
    const TileTypeLookup& tileTypeLookup)
{
    if (!hasRoadOverlayAt(coordinates, tileType, network))
    {
        return false;
    }

    return !isClippedSingleTileRoadEndCap(
        getNeighborRoadOverlayOccupancy(
            coordinates,
            network,
            tileTypeLookup));
}

template <typename TileTypeLookup>
[[nodiscard]] inline bool hasPublishedRoadOverlayAt(
    const TileCoordinates& coordinates,
    const TileType tileType,
    const TileCoordinates& spawnTile,
    const std::uint32_t seed,
    const TileTypeLookup& tileTypeLookup)
{
    return hasPublishedRoadOverlayAt(
        coordinates,
        tileType,
        buildRoadNetwork(spawnTile, seed),
        tileTypeLookup);
}

} // namespace detail
} // namespace rpg

#endif // RPG_MAIN_ROAD_NETWORK_SUPPORT_HPP
