/**
 * @file RoadStampSupport.hpp
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

#ifndef RPG_MAIN_ROAD_STAMP_SUPPORT_HPP
#define RPG_MAIN_ROAD_STAMP_SUPPORT_HPP

#include "RoadNetworkSupport.hpp"

#include <algorithm>
#include <array>

namespace rpg
{
namespace detail
{

struct RoadStampedTile
{
    bool isRoad = false;
    bool isPublished = false;
    bool isShoulder = false;
    bool isIntersection = false;
    int stampedWidth = 0;
    std::array<bool, 8> publishedNeighborOccupancy{};
};

namespace
{

struct RoadStampState
{
    bool carriage = false;
    bool shoulder = false;
    bool intersection = false;
    int stampedWidth = 0;
};

[[nodiscard]] inline bool occupiesAnchoredSquare(
    const TileCoordinates& coordinates,
    const TileCoordinates& anchor,
    const int width,
    const int shoulderWidth) noexcept
{
    if (width <= 0)
    {
        return false;
    }

    const int minimumX = anchor.x - shoulderWidth;
    const int maximumX = anchor.x + width + shoulderWidth - 1;
    const int minimumY = anchor.y - shoulderWidth;
    const int maximumY = anchor.y + width + shoulderWidth - 1;
    return coordinates.x >= minimumX
        && coordinates.x <= maximumX
        && coordinates.y >= minimumY
        && coordinates.y <= maximumY;
}

[[nodiscard]] inline bool occupiesAnchoredSquareCore(
    const TileCoordinates& coordinates,
    const TileCoordinates& anchor,
    const int width) noexcept
{
    return occupiesAnchoredSquare(coordinates, anchor, width, 0);
}

[[nodiscard]] inline bool occupiesHorizontalStampedRun(
    const TileCoordinates& coordinates,
    const TileCoordinates& start,
    const TileCoordinates& end,
    const int width,
    const int shoulderWidth) noexcept
{
    if (width <= 0 || start.y != end.y)
    {
        return false;
    }

    const int minimumX = std::min(start.x, end.x);
    const int maximumX = std::max(start.x, end.x);
    const int minimumY = start.y - shoulderWidth;
    const int maximumY = start.y + width + shoulderWidth - 1;
    return coordinates.x >= minimumX
        && coordinates.x <= maximumX
        && coordinates.y >= minimumY
        && coordinates.y <= maximumY;
}

[[nodiscard]] inline bool occupiesVerticalStampedRun(
    const TileCoordinates& coordinates,
    const TileCoordinates& start,
    const TileCoordinates& end,
    const int width,
    const int shoulderWidth) noexcept
{
    if (width <= 0 || start.x != end.x)
    {
        return false;
    }

    const int minimumX = start.x - shoulderWidth;
    const int maximumX = start.x + width + shoulderWidth - 1;
    const int minimumY = std::min(start.y, end.y);
    const int maximumY = std::max(start.y, end.y);
    return coordinates.x >= minimumX
        && coordinates.x <= maximumX
        && coordinates.y >= minimumY
        && coordinates.y <= maximumY;
}

[[nodiscard]] inline bool occupiesStampedRun(
    const TileCoordinates& coordinates,
    const TileCoordinates& start,
    const TileCoordinates& end,
    const int width,
    const int shoulderWidth) noexcept
{
    return occupiesHorizontalStampedRun(coordinates, start, end, width, shoulderWidth)
        || occupiesVerticalStampedRun(coordinates, start, end, width, shoulderWidth);
}

[[nodiscard]] inline bool occupiesRoadSegmentBendFootprint(
    const TileCoordinates& coordinates,
    const RoadSegment& segment,
    const int shoulderWidth) noexcept
{
    if (!segment.stampMetadata.stampBends || segment.polyline.size() < 3U || segment.stampMetadata.carriageWidth <= 0)
    {
        return false;
    }

    for (std::size_t pointIndex = 1U; pointIndex + 1U < segment.polyline.size(); ++pointIndex)
    {
        if (occupiesAnchoredSquare(
                coordinates,
                segment.polyline[pointIndex],
                segment.stampMetadata.carriageWidth,
                shoulderWidth))
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] inline RoadStampState collectRoadStampState(
    const TileCoordinates& coordinates,
    const RoadNetwork& network) noexcept
{
    RoadStampState state;

    for (const RoadNode& node : network.nodes)
    {
        const int footprintWidth = std::max(node.stampMetadata.footprintWidth, 1);

        if (occupiesAnchoredSquareCore(coordinates, node.coordinates, footprintWidth))
        {
            state.carriage = true;
            state.intersection = state.intersection || node.kind != RoadNodeKind::Destination;
            state.stampedWidth = std::max(state.stampedWidth, footprintWidth);
        }
        else if (node.stampMetadata.shoulderWidth > 0
                 && occupiesAnchoredSquare(
                     coordinates,
                     node.coordinates,
                     footprintWidth,
                     node.stampMetadata.shoulderWidth))
        {
            state.shoulder = true;
            state.intersection = state.intersection || node.kind != RoadNodeKind::Destination;
            state.stampedWidth = std::max(
                state.stampedWidth,
                footprintWidth + (node.stampMetadata.shoulderWidth * 2));
        }
    }

    for (const RoadSegment& segment : network.segments)
    {
        if (segment.polyline.size() < 2U)
        {
            continue;
        }

        const int carriageWidth = std::max(segment.stampMetadata.carriageWidth, 1);

        for (std::size_t pointIndex = 1U; pointIndex < segment.polyline.size(); ++pointIndex)
        {
            if (occupiesStampedRun(
                    coordinates,
                    segment.polyline[pointIndex - 1U],
                    segment.polyline[pointIndex],
                    carriageWidth,
                    0))
            {
                state.carriage = true;
                state.stampedWidth = std::max(state.stampedWidth, carriageWidth);
            }
            else if (segment.stampMetadata.shoulderWidth > 0
                     && occupiesStampedRun(
                         coordinates,
                         segment.polyline[pointIndex - 1U],
                         segment.polyline[pointIndex],
                         carriageWidth,
                         segment.stampMetadata.shoulderWidth))
            {
                state.shoulder = true;
                state.stampedWidth = std::max(
                    state.stampedWidth,
                    carriageWidth + (segment.stampMetadata.shoulderWidth * 2));
            }
        }

        if (occupiesRoadSegmentBendFootprint(coordinates, segment, 0))
        {
            state.carriage = true;
            state.intersection = true;
            state.stampedWidth = std::max(state.stampedWidth, carriageWidth);
        }
        else if (segment.stampMetadata.shoulderWidth > 0
                 && occupiesRoadSegmentBendFootprint(
                     coordinates,
                     segment,
                     segment.stampMetadata.shoulderWidth))
        {
            state.shoulder = true;
            state.intersection = true;
            state.stampedWidth = std::max(
                state.stampedWidth,
                carriageWidth + (segment.stampMetadata.shoulderWidth * 2));
        }
    }

    return state;
}

[[nodiscard]] inline bool occupiesStampedRoadTile(
    const TileCoordinates& coordinates,
    const RoadNetwork& network) noexcept
{
    const RoadStampState state = collectRoadStampState(coordinates, network);
    return state.carriage || state.shoulder;
}

} // namespace

[[nodiscard]] inline bool hasStampedRoadOverlayAt(
    const TileCoordinates& coordinates,
    const TileType tileType,
    const RoadNetwork& network) noexcept
{
    return supportsRoadOverlaySurface(tileType) && occupiesStampedRoadTile(coordinates, network);
}

template <typename TileTypeLookup>
[[nodiscard]] inline std::array<bool, 8> getStampedNeighborRoadOverlayOccupancy(
    const TileCoordinates& coordinates,
    const RoadNetwork& network,
    const TileTypeLookup& tileTypeLookup)
{
    const auto hasNeighborRoadOverlay = [&network, &tileTypeLookup](const int x, const int y)
    {
        const TileCoordinates neighborCoordinates{x, y};
        return hasStampedRoadOverlayAt(
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
[[nodiscard]] inline bool hasPublishedStampedRoadOverlayAt(
    const TileCoordinates& coordinates,
    const TileType tileType,
    const RoadNetwork& network,
    const TileTypeLookup& tileTypeLookup)
{
    if (!hasStampedRoadOverlayAt(coordinates, tileType, network))
    {
        return false;
    }

    return !isClippedSingleTileRoadEndCap(
        getStampedNeighborRoadOverlayOccupancy(
            coordinates,
            network,
            tileTypeLookup));
}

template <typename TileTypeLookup>
[[nodiscard]] inline std::array<bool, 8> getPublishedStampedNeighborRoadOverlayOccupancy(
    const TileCoordinates& coordinates,
    const RoadNetwork& network,
    const TileTypeLookup& tileTypeLookup)
{
    const auto hasNeighborRoadOverlay = [&network, &tileTypeLookup](const int x, const int y)
    {
        const TileCoordinates neighborCoordinates{x, y};
        return hasPublishedStampedRoadOverlayAt(
            neighborCoordinates,
            tileTypeLookup(neighborCoordinates),
            network,
            tileTypeLookup);
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
[[nodiscard]] inline RoadStampedTile getRoadStampedTile(
    const TileCoordinates& coordinates,
    const TileType tileType,
    const RoadNetwork& network,
    const TileTypeLookup& tileTypeLookup)
{
    RoadStampedTile stampedTile;

    if (!hasStampedRoadOverlayAt(coordinates, tileType, network))
    {
        return stampedTile;
    }

    const RoadStampState state = collectRoadStampState(coordinates, network);
    stampedTile.isRoad = true;
    stampedTile.isShoulder = state.shoulder && !state.carriage;
    stampedTile.stampedWidth = state.stampedWidth;
    stampedTile.isPublished = hasPublishedStampedRoadOverlayAt(coordinates, tileType, network, tileTypeLookup);

    if (stampedTile.isPublished)
    {
        stampedTile.publishedNeighborOccupancy = getPublishedStampedNeighborRoadOverlayOccupancy(
            coordinates,
            network,
            tileTypeLookup);
        const int cardinalNeighborCount =
            static_cast<int>(stampedTile.publishedNeighborOccupancy[0])
            + static_cast<int>(stampedTile.publishedNeighborOccupancy[2])
            + static_cast<int>(stampedTile.publishedNeighborOccupancy[4])
            + static_cast<int>(stampedTile.publishedNeighborOccupancy[6]);
        stampedTile.isIntersection = state.intersection || cardinalNeighborCount >= 3;
        return stampedTile;
    }

    stampedTile.isIntersection = state.intersection;
    return stampedTile;
}

} // namespace detail
} // namespace rpg

#endif // RPG_MAIN_ROAD_STAMP_SUPPORT_HPP
