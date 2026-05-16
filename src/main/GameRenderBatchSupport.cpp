/**
 * @file GameRenderBatchSupport.cpp
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

#include "GameRenderBatchSupport.hpp"

#include <SFML/Graphics/Rect.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace rpg::detail
{

namespace
{

constexpr int kTerrainTilesetCellSize = 16;
constexpr int kRoadOverlayTilesetCellSize = 16;
constexpr std::size_t kVerticesPerQuad = 6U;
constexpr std::uint32_t kRoadOverlayBaseVariantHashSalt = 0xA5F13C9DU;
constexpr std::uint32_t kRoadOverlayDecorRollHashSalt = 0x5C2D91EBU;
constexpr std::uint32_t kRoadOverlayDecorVariantHashSalt = 0xC97F4A1DU;
constexpr std::uint32_t kRoadOverlayDecorWeightNumerator = 1U;
constexpr std::uint32_t kRoadOverlayDecorWeightDenominator = 8U;

struct VisibleTileGridBounds
{
    int minTileX = 0;
    int maxTileX = 0;
    int minTileY = 0;
    int maxTileY = 0;
    float left = 0.0F;
    float top = 0.0F;
    float right = 0.0F;
    float bottom = 0.0F;
    float tileWidth = 0.0F;
    float tileHeight = 0.0F;
};

[[nodiscard]] sf::IntRect getTerrainTilesetRect(const TerrainAtlasCell& cell) noexcept
{
    return {
        {cell.tileX * kTerrainTilesetCellSize, cell.tileY * kTerrainTilesetCellSize},
        {kTerrainTilesetCellSize, kTerrainTilesetCellSize}};
}

[[nodiscard]] sf::IntRect getRoadOverlayTilesetRect(const RoadOverlayAtlasCell& cell) noexcept
{
    return {
        {cell.tileX * kRoadOverlayTilesetCellSize, cell.tileY * kRoadOverlayTilesetCellSize},
        {kRoadOverlayTilesetCellSize, kRoadOverlayTilesetCellSize}};
}

[[nodiscard]] std::uint64_t packTileCoordinates(const int x, const int y) noexcept
{
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32U)
        | static_cast<std::uint32_t>(y);
}

using VisibleTileTypeMap = std::unordered_map<std::uint64_t, TileType>;
using VisibleRoadOverlaySurfaceMap = std::unordered_map<std::uint64_t, TileType>;

[[nodiscard]] VisibleTileTypeMap buildVisibleTileTypeMap(const OverworldRenderSnapshot& renderSnapshot)
{
    VisibleTileTypeMap visibleTileTypes;
    visibleTileTypes.reserve(renderSnapshot.visibleTiles.size());

    for (const OverworldRenderTile& visibleTile : renderSnapshot.visibleTiles)
    {
        visibleTileTypes.emplace(
            packTileCoordinates(visibleTile.coordinates.x, visibleTile.coordinates.y),
            visibleTile.tileType);
    }

    return visibleTileTypes;
}

[[nodiscard]] VisibleRoadOverlaySurfaceMap buildVisibleRoadOverlaySurfaceMap(const OverworldRenderSnapshot& renderSnapshot)
{
    VisibleRoadOverlaySurfaceMap visibleRoadOverlaySurfaces;
    visibleRoadOverlaySurfaces.reserve(renderSnapshot.visibleRoadOverlays.size());

    for (const OverworldRenderRoadOverlay& roadOverlay : renderSnapshot.visibleRoadOverlays)
    {
        visibleRoadOverlaySurfaces.emplace(
            packTileCoordinates(roadOverlay.coordinates.x, roadOverlay.coordinates.y),
            roadOverlay.surfaceTileType);
    }

    return visibleRoadOverlaySurfaces;
}

[[nodiscard]] std::array<TileType, 8> getNeighborTileTypes(
    const VisibleTileTypeMap& visibleTileTypes,
    const OverworldRenderTile& visibleTile) noexcept
{
    const auto getTileTypeOrCurrent = [&visibleTileTypes, &visibleTile](const int x, const int y) noexcept
    {
        const auto tileIt = visibleTileTypes.find(packTileCoordinates(x, y));
        return tileIt == visibleTileTypes.end() ? visibleTile.tileType : tileIt->second;
    };

    return {
        getTileTypeOrCurrent(visibleTile.coordinates.x, visibleTile.coordinates.y - 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x + 1, visibleTile.coordinates.y - 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x + 1, visibleTile.coordinates.y),
        getTileTypeOrCurrent(visibleTile.coordinates.x + 1, visibleTile.coordinates.y + 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x, visibleTile.coordinates.y + 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x - 1, visibleTile.coordinates.y + 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x - 1, visibleTile.coordinates.y),
        getTileTypeOrCurrent(visibleTile.coordinates.x - 1, visibleTile.coordinates.y - 1)};
}

[[nodiscard]] std::array<bool, 8> getNeighborRoadOverlayOccupancy(
    const VisibleRoadOverlaySurfaceMap& visibleRoadOverlaySurfaces,
    const OverworldRenderRoadOverlay& roadOverlay) noexcept
{
    const auto hasRoadOverlay = [&visibleRoadOverlaySurfaces](const int x, const int y) noexcept
    {
        return visibleRoadOverlaySurfaces.contains(packTileCoordinates(x, y));
    };

    return {
        hasRoadOverlay(roadOverlay.coordinates.x, roadOverlay.coordinates.y - 1),
        hasRoadOverlay(roadOverlay.coordinates.x + 1, roadOverlay.coordinates.y - 1),
        hasRoadOverlay(roadOverlay.coordinates.x + 1, roadOverlay.coordinates.y),
        hasRoadOverlay(roadOverlay.coordinates.x + 1, roadOverlay.coordinates.y + 1),
        hasRoadOverlay(roadOverlay.coordinates.x, roadOverlay.coordinates.y + 1),
        hasRoadOverlay(roadOverlay.coordinates.x - 1, roadOverlay.coordinates.y + 1),
        hasRoadOverlay(roadOverlay.coordinates.x - 1, roadOverlay.coordinates.y),
        hasRoadOverlay(roadOverlay.coordinates.x - 1, roadOverlay.coordinates.y - 1)};
}

[[nodiscard]] int getRoadOverlaySurfacePriority(const TileType tileType) noexcept
{
    switch (tileType)
    {
    case TileType::Grass:
        return 1;
    case TileType::Forest:
        return 2;
    case TileType::Sand:
        return 3;
    case TileType::Water:
        return 0;
    }

    return 0;
}

[[nodiscard]] std::uint32_t hashRoadOverlaySelection(
    const std::uint32_t seed,
    const TileCoordinates& coordinates,
    const TileType surfaceTileType,
    const std::uint32_t salt) noexcept
{
    std::uint32_t value = seed ^ salt;
    value ^= static_cast<std::uint32_t>(coordinates.x) * 73856093U;
    value ^= static_cast<std::uint32_t>(coordinates.y) * 19349663U;
    value ^= static_cast<std::uint32_t>(getRoadOverlaySurfacePriority(surfaceTileType)) * 83492791U;
    value ^= value >> 13U;
    value *= 1274126177U;
    value ^= value >> 16U;
    return value;
}

struct RoadOverlayAppearanceSelection
{
    bool useDecor = false;
    std::size_t variantIndex = 0U;
};

[[nodiscard]] std::size_t selectRoadOverlayVariantIndex(
    const std::uint32_t seed,
    const TileCoordinates& coordinates,
    const TileType surfaceTileType,
    const std::size_t variantCount,
    const std::uint32_t salt) noexcept
{
    if (variantCount == 0U)
    {
        return 0U;
    }

    return static_cast<std::size_t>(hashRoadOverlaySelection(seed, coordinates, surfaceTileType, salt) % variantCount);
}

[[nodiscard]] RoadOverlayAppearanceSelection selectRoadOverlayAppearanceSelection(
    const std::uint32_t seed,
    const TileCoordinates& coordinates,
    const TileType surfaceTileType,
    const std::size_t baseVariantCount,
    const std::size_t decorVariantCount) noexcept
{
    RoadOverlayAppearanceSelection selection;

    if (baseVariantCount == 0U)
    {
        return selection;
    }

    selection.useDecor = decorVariantCount > 0U
        && (hashRoadOverlaySelection(seed, coordinates, surfaceTileType, kRoadOverlayDecorRollHashSalt)
            % kRoadOverlayDecorWeightDenominator) < kRoadOverlayDecorWeightNumerator;
    selection.variantIndex = selectRoadOverlayVariantIndex(
        seed,
        coordinates,
        surfaceTileType,
        selection.useDecor ? decorVariantCount : baseVariantCount,
        selection.useDecor ? kRoadOverlayDecorVariantHashSalt : kRoadOverlayBaseVariantHashSalt);
    return selection;
}

[[nodiscard]] std::optional<RoadOverlayAutotileRole> resolveRoadOverlayRole(
    const std::array<bool, 8>& occupancy) noexcept
{
    const bool north = occupancy[0];
    const bool northEast = occupancy[1];
    const bool east = occupancy[2];
    const bool southEast = occupancy[3];
    const bool south = occupancy[4];
    const bool southWest = occupancy[5];
    const bool west = occupancy[6];
    const bool northWest = occupancy[7];

    if (north && east && south && west)
    {
        if (!northWest)
        {
            return RoadOverlayAutotileRole::OuterTopLeft;
        }

        if (!northEast)
        {
            return RoadOverlayAutotileRole::OuterTopRight;
        }

        if (!southWest)
        {
            return RoadOverlayAutotileRole::OuterBottomLeft;
        }

        if (!southEast)
        {
            return RoadOverlayAutotileRole::OuterBottomRight;
        }

        return std::nullopt;
    }

    if (!north && east && south && west)
    {
        return RoadOverlayAutotileRole::Top;
    }

    if (!south && north && east && west)
    {
        return RoadOverlayAutotileRole::Bottom;
    }

    if (!west && north && east && south)
    {
        return RoadOverlayAutotileRole::Left;
    }

    if (!east && north && south && west)
    {
        return RoadOverlayAutotileRole::Right;
    }

    if (!north && !west && east && south)
    {
        return RoadOverlayAutotileRole::TopLeft;
    }

    if (!north && !east && west && south)
    {
        return RoadOverlayAutotileRole::TopRight;
    }

    if (!south && !west && north && east)
    {
        return RoadOverlayAutotileRole::BottomLeft;
    }

    if (!south && !east && north && west)
    {
        return RoadOverlayAutotileRole::BottomRight;
    }

    if (!north && !east && !south && southWest && west && northWest)
    {
        return RoadOverlayAutotileRole::Right;
    }

    if (!north && northEast && east && southEast && !south && !west)
    {
        return RoadOverlayAutotileRole::Left;
    }

    if (!north && !east && southEast && south && southWest && !west)
    {
        return RoadOverlayAutotileRole::Top;
    }

    if (north && northEast && !east && !south && !west && northWest)
    {
        return RoadOverlayAutotileRole::Bottom;
    }

    return std::nullopt;
}

[[nodiscard]] RoadOverlayAtlasCell selectRoadOverlayAtlasCell(
    const RoadOverlayTilesetMetadata& metadata,
    const OverworldRenderRoadOverlay& roadOverlay,
    const std::array<bool, 8>& occupancy,
    const std::uint32_t seed)
{
    const std::optional<RoadOverlayAutotileRole> role = resolveRoadOverlayRole(occupancy);

    if (!role.has_value())
    {
        const RoadOverlayAppearanceSelection selection = selectRoadOverlayAppearanceSelection(
            seed,
            roadOverlay.coordinates,
            roadOverlay.surfaceTileType,
            metadata.getBaseVariantCount(),
            metadata.getDecorVariantCount());
        return selection.useDecor
            ? metadata.getDecorVariant(selection.variantIndex)
            : metadata.getBaseVariant(selection.variantIndex);
    }

    return metadata.getTransitionCell(roadOverlay.surfaceTileType, *role);
}

void setTexturedQuadVertices(
    sf::VertexArray& vertexArray,
    const std::size_t startIndex,
    const float left,
    const float top,
    const float right,
    const float bottom,
    const sf::IntRect& textureRect)
{
    const float textureLeft = static_cast<float>(textureRect.position.x);
    const float textureTop = static_cast<float>(textureRect.position.y);
    const float textureRight = textureLeft + static_cast<float>(textureRect.size.x);
    const float textureBottom = textureTop + static_cast<float>(textureRect.size.y);

    vertexArray[startIndex + 0U] = {{left, top}, sf::Color::White, {textureLeft, textureTop}};
    vertexArray[startIndex + 1U] = {{left, bottom}, sf::Color::White, {textureLeft, textureBottom}};
    vertexArray[startIndex + 2U] = {{right, bottom}, sf::Color::White, {textureRight, textureBottom}};
    vertexArray[startIndex + 3U] = {{left, top}, sf::Color::White, {textureLeft, textureTop}};
    vertexArray[startIndex + 4U] = {{right, bottom}, sf::Color::White, {textureRight, textureBottom}};
    vertexArray[startIndex + 5U] = {{right, top}, sf::Color::White, {textureRight, textureTop}};
}

void setColoredQuadVertices(
    sf::VertexArray& vertexArray,
    const std::size_t startIndex,
    const OverlayRectangle& rectangle,
    const sf::Color& color)
{
    const float left = rectangle.position.x;
    const float top = rectangle.position.y;
    const float right = left + rectangle.size.width;
    const float bottom = top + rectangle.size.height;

    vertexArray[startIndex + 0U] = {{left, top}, color};
    vertexArray[startIndex + 1U] = {{left, bottom}, color};
    vertexArray[startIndex + 2U] = {{right, bottom}, color};
    vertexArray[startIndex + 3U] = {{left, top}, color};
    vertexArray[startIndex + 4U] = {{right, bottom}, color};
    vertexArray[startIndex + 5U] = {{right, top}, color};
}

[[nodiscard]] VisibleTileGridBounds getVisibleTileGridBounds(const std::vector<OverworldRenderTile>& visibleTiles) noexcept
{
    const OverworldRenderTile& firstVisibleTile = visibleTiles.front();
    const float firstLeft = firstVisibleTile.position.x - firstVisibleTile.origin.x;
    const float firstTop = firstVisibleTile.position.y - firstVisibleTile.origin.y;
    VisibleTileGridBounds bounds{
        firstVisibleTile.coordinates.x,
        firstVisibleTile.coordinates.x,
        firstVisibleTile.coordinates.y,
        firstVisibleTile.coordinates.y,
        firstLeft,
        firstTop,
        firstLeft + firstVisibleTile.size.width,
        firstTop + firstVisibleTile.size.height,
        firstVisibleTile.size.width,
        firstVisibleTile.size.height};

    for (const OverworldRenderTile& visibleTile : visibleTiles)
    {
        assert(visibleTile.size.width == bounds.tileWidth && visibleTile.size.height == bounds.tileHeight);
        const float left = visibleTile.position.x - visibleTile.origin.x;
        const float top = visibleTile.position.y - visibleTile.origin.y;
        const float right = left + visibleTile.size.width;
        const float bottom = top + visibleTile.size.height;

        if (visibleTile.coordinates.x < bounds.minTileX)
        {
            bounds.minTileX = visibleTile.coordinates.x;
        }

        if (visibleTile.coordinates.x > bounds.maxTileX)
        {
            bounds.maxTileX = visibleTile.coordinates.x;
        }

        if (visibleTile.coordinates.y < bounds.minTileY)
        {
            bounds.minTileY = visibleTile.coordinates.y;
        }

        if (visibleTile.coordinates.y > bounds.maxTileY)
        {
            bounds.maxTileY = visibleTile.coordinates.y;
        }

        if (left < bounds.left)
        {
            bounds.left = left;
        }

        if (top < bounds.top)
        {
            bounds.top = top;
        }

        if (right > bounds.right)
        {
            bounds.right = right;
        }

        if (bottom > bounds.bottom)
        {
            bounds.bottom = bottom;
        }
    }

    const std::size_t expectedVisibleTileCount = static_cast<std::size_t>(bounds.maxTileX - bounds.minTileX + 1)
        * static_cast<std::size_t>(bounds.maxTileY - bounds.minTileY + 1);
    assert(expectedVisibleTileCount == visibleTiles.size());

    return bounds;
}

[[nodiscard]] OverlayRectangle makeHorizontalGridStrip(
    const VisibleTileGridBounds& bounds,
    const int boundaryIndex,
    const float lineThickness) noexcept
{
    const int rowCount = bounds.maxTileY - bounds.minTileY + 1;
    const bool isOuterBoundary = boundaryIndex == 0 || boundaryIndex == rowCount;
    const float top = boundaryIndex == 0
        ? bounds.top
        : (boundaryIndex == rowCount
              ? bounds.bottom - lineThickness
              : bounds.top + static_cast<float>(boundaryIndex) * bounds.tileHeight - lineThickness);

    return {
        {bounds.right - bounds.left, isOuterBoundary ? lineThickness : lineThickness * 2.0F},
        {bounds.left, top}};
}

[[nodiscard]] OverlayRectangle makeVerticalGridStrip(
    const VisibleTileGridBounds& bounds,
    const int boundaryIndex,
    const float lineThickness) noexcept
{
    const int columnCount = bounds.maxTileX - bounds.minTileX + 1;
    const bool isOuterBoundary = boundaryIndex == 0 || boundaryIndex == columnCount;
    const float left = boundaryIndex == 0
        ? bounds.left
        : (boundaryIndex == columnCount
              ? bounds.right - lineThickness
              : bounds.left + static_cast<float>(boundaryIndex) * bounds.tileWidth - lineThickness);

    return {
        {isOuterBoundary ? lineThickness : lineThickness * 2.0F, bounds.bottom - bounds.top},
        {left, bounds.top}};
}

} // namespace

sf::VertexArray buildTerrainVertexArray(
    const TerrainTilesetMetadata& terrainTilesetMetadata,
    const OverworldRenderSnapshot& renderSnapshot,
    const float animationElapsedSeconds,
    const std::uint32_t seed)
{
    sf::VertexArray terrainVertexArray(sf::PrimitiveType::Triangles);

    if (renderSnapshot.visibleTiles.empty())
    {
        return terrainVertexArray;
    }

    terrainVertexArray.resize(renderSnapshot.visibleTiles.size() * kVerticesPerQuad);
    const VisibleTileTypeMap visibleTileTypes = buildVisibleTileTypeMap(renderSnapshot);
    std::size_t vertexIndex = 0U;

    for (const OverworldRenderTile& visibleTile : renderSnapshot.visibleTiles)
    {
        const sf::IntRect textureRect = getTerrainTilesetRect(selectTerrainAtlasCell(
            terrainTilesetMetadata,
            visibleTile,
            getNeighborTileTypes(visibleTileTypes, visibleTile),
            animationElapsedSeconds,
            seed));
        const float left = visibleTile.position.x - visibleTile.origin.x;
        const float top = visibleTile.position.y - visibleTile.origin.y;
        const float right = left + visibleTile.size.width;
        const float bottom = top + visibleTile.size.height;
        setTexturedQuadVertices(terrainVertexArray, vertexIndex, left, top, right, bottom, textureRect);
        vertexIndex += kVerticesPerQuad;
    }

    return terrainVertexArray;
}

sf::VertexArray buildRoadOverlayVertexArray(
    const RoadOverlayTilesetMetadata& roadOverlayTilesetMetadata,
    const OverworldRenderSnapshot& renderSnapshot,
    const std::uint32_t seed)
{
    sf::VertexArray roadOverlayVertexArray(sf::PrimitiveType::Triangles);

    if (renderSnapshot.visibleRoadOverlays.empty())
    {
        return roadOverlayVertexArray;
    }

    roadOverlayVertexArray.resize(renderSnapshot.visibleRoadOverlays.size() * kVerticesPerQuad);
    const VisibleRoadOverlaySurfaceMap visibleRoadOverlaySurfaces = buildVisibleRoadOverlaySurfaceMap(renderSnapshot);
    std::size_t vertexIndex = 0U;

    for (const OverworldRenderRoadOverlay& roadOverlay : renderSnapshot.visibleRoadOverlays)
    {
        const sf::IntRect textureRect = getRoadOverlayTilesetRect(selectRoadOverlayAtlasCell(
            roadOverlayTilesetMetadata,
            roadOverlay,
            getNeighborRoadOverlayOccupancy(visibleRoadOverlaySurfaces, roadOverlay),
            seed));
        const float left = roadOverlay.position.x - roadOverlay.origin.x;
        const float top = roadOverlay.position.y - roadOverlay.origin.y;
        const float right = left + roadOverlay.size.width;
        const float bottom = top + roadOverlay.size.height;
        setTexturedQuadVertices(roadOverlayVertexArray, vertexIndex, left, top, right, bottom, textureRect);
        vertexIndex += kVerticesPerQuad;
    }

    return roadOverlayVertexArray;
}

sf::VertexArray buildTileGridVertexArray(
    const std::vector<OverworldRenderTile>& visibleTiles,
    const float lineThickness,
    const sf::Color& lineColor)
{
    sf::VertexArray tileGridVertexArray(sf::PrimitiveType::Triangles);

    if (visibleTiles.empty() || lineThickness <= 0.0F)
    {
        return tileGridVertexArray;
    }

    const VisibleTileGridBounds bounds = getVisibleTileGridBounds(visibleTiles);
    const std::size_t horizontalStripCount = static_cast<std::size_t>(bounds.maxTileY - bounds.minTileY + 2);
    const std::size_t verticalStripCount = static_cast<std::size_t>(bounds.maxTileX - bounds.minTileX + 2);
    tileGridVertexArray.resize((horizontalStripCount + verticalStripCount) * kVerticesPerQuad);
    std::size_t vertexIndex = 0U;

    for (int boundaryIndex = 0; boundaryIndex < bounds.maxTileY - bounds.minTileY + 2; ++boundaryIndex)
    {
        setColoredQuadVertices(
            tileGridVertexArray,
            vertexIndex,
            makeHorizontalGridStrip(bounds, boundaryIndex, lineThickness),
            lineColor);
        vertexIndex += kVerticesPerQuad;
    }

    for (int boundaryIndex = 0; boundaryIndex < bounds.maxTileX - bounds.minTileX + 2; ++boundaryIndex)
    {
        setColoredQuadVertices(
            tileGridVertexArray,
            vertexIndex,
            makeVerticalGridStrip(bounds, boundaryIndex, lineThickness),
            lineColor);
        vertexIndex += kVerticesPerQuad;
    }

    return tileGridVertexArray;
}

} // namespace rpg::detail
