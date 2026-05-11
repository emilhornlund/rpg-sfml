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
#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace rpg::detail
{

namespace
{

constexpr int kTerrainTilesetCellSize = 16;
constexpr std::size_t kVerticesPerQuad = 6U;
constexpr std::size_t kGridRectanglesPerTile = 4U;

[[nodiscard]] sf::IntRect getTerrainTilesetRect(const TerrainAtlasCell& cell) noexcept
{
    return {
        {cell.tileX * kTerrainTilesetCellSize, cell.tileY * kTerrainTilesetCellSize},
        {kTerrainTilesetCellSize, kTerrainTilesetCellSize}};
}

[[nodiscard]] std::uint64_t packTileCoordinates(const int x, const int y) noexcept
{
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32U)
        | static_cast<std::uint32_t>(y);
}

using VisibleTileTypeMap = std::unordered_map<std::uint64_t, TileType>;

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

    tileGridVertexArray.resize(visibleTiles.size() * kGridRectanglesPerTile * kVerticesPerQuad);
    std::size_t vertexIndex = 0U;

    for (const OverworldRenderTile& visibleTile : visibleTiles)
    {
        const auto overlayRectangles = getTileGridOverlayRectangles(visibleTile, lineThickness);

        for (const OverlayRectangle& overlayRectangle : overlayRectangles)
        {
            setColoredQuadVertices(tileGridVertexArray, vertexIndex, overlayRectangle, lineColor);
            vertexIndex += kVerticesPerQuad;
        }
    }

    return tileGridVertexArray;
}

} // namespace rpg::detail
