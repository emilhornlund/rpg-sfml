/**
 * @file GameRenderBatchSupportTests.cpp
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

#include "GameAssetSupport.hpp"
#include "GameRenderBatchSupport.hpp"

#include <SFML/Graphics/Rect.hpp>

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace
{

[[nodiscard]] bool areClose(const float lhs, const float rhs) noexcept
{
    return std::fabs(lhs - rhs) < 0.0001F;
}

[[nodiscard]] bool matchesVertex(
    const sf::Vertex& vertex,
    const float expectedX,
    const float expectedY,
    const sf::Color& expectedColor) noexcept
{
    return areClose(vertex.position.x, expectedX)
        && areClose(vertex.position.y, expectedY)
        && vertex.color == expectedColor;
}

[[nodiscard]] bool matchesTexturedVertex(
    const sf::Vertex& vertex,
    const float expectedX,
    const float expectedY,
    const sf::Color& expectedColor,
    const float expectedTextureX,
    const float expectedTextureY) noexcept
{
    return areClose(vertex.position.x, expectedX)
        && areClose(vertex.position.y, expectedY)
        && vertex.color == expectedColor
        && areClose(vertex.texCoords.x, expectedTextureX)
        && areClose(vertex.texCoords.y, expectedTextureY);
}

[[nodiscard]] sf::IntRect makeTerrainTilesetRect(const rpg::detail::TerrainAtlasCell& cell) noexcept
{
    return {
        {cell.tileX * 16, cell.tileY * 16},
        {16, 16}};
}

[[nodiscard]] rpg::OverworldRenderTile makeVisibleTile(const int x, const int y, const rpg::TileType tileType) noexcept
{
    return {
        {x, y},
        tileType,
        {16.0F, 16.0F},
        {8.0F, 8.0F},
        {(static_cast<float>(x) + 0.5F) * 16.0F, (static_cast<float>(y) + 0.5F) * 16.0F}};
}

bool verifyTerrainBatchUsesExpectedGeometry(const std::filesystem::path& assetRoot)
{
    const rpg::detail::TerrainTilesetMetadata metadata = rpg::detail::loadTerrainTilesetMetadata(assetRoot);
    rpg::OverworldRenderSnapshot renderSnapshot;
    renderSnapshot.visibleTiles = {
        makeVisibleTile(0, 0, rpg::TileType::Grass),
        makeVisibleTile(1, 0, rpg::TileType::Grass)};
    constexpr std::uint32_t kSeed = 0x00C0FFEEU;
    constexpr std::array<rpg::TileType, 8> kGrassNeighbors{
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass};

    const sf::VertexArray terrainVertexArray =
        rpg::detail::buildTerrainVertexArray(metadata, renderSnapshot, 0.0F, kSeed);
    const sf::IntRect textureRect = makeTerrainTilesetRect(rpg::detail::selectTerrainAtlasCell(
        metadata,
        renderSnapshot.visibleTiles.front(),
        kGrassNeighbors,
        0.0F,
        kSeed));

    return terrainVertexArray.getPrimitiveType() == sf::PrimitiveType::Triangles
        && terrainVertexArray.getVertexCount() == 12U
        && matchesTexturedVertex(
            terrainVertexArray[0],
            0.0F,
            0.0F,
            sf::Color::White,
            static_cast<float>(textureRect.position.x),
            static_cast<float>(textureRect.position.y))
        && matchesTexturedVertex(
            terrainVertexArray[1],
            0.0F,
            16.0F,
            sf::Color::White,
            static_cast<float>(textureRect.position.x),
            static_cast<float>(textureRect.position.y + textureRect.size.y))
        && matchesTexturedVertex(
            terrainVertexArray[2],
            16.0F,
            16.0F,
            sf::Color::White,
            static_cast<float>(textureRect.position.x + textureRect.size.x),
            static_cast<float>(textureRect.position.y + textureRect.size.y))
        && matchesVertex(terrainVertexArray[6], 16.0F, 0.0F, sf::Color::White)
        && matchesVertex(terrainVertexArray[11], 32.0F, 0.0F, sf::Color::White);
}

bool verifyTileGridBatchUsesOverlayRectangles()
{
    const rpg::OverworldRenderTile visibleTile = makeVisibleTile(2, 3, rpg::TileType::Forest);
    const sf::Color gridColor(255, 255, 255, 96);
    const sf::VertexArray tileGridVertexArray =
        rpg::detail::buildTileGridVertexArray(std::vector<rpg::OverworldRenderTile>{visibleTile}, 1.0F, gridColor);
    const auto overlayRectangles = rpg::detail::getTileGridOverlayRectangles(visibleTile, 1.0F);

    return tileGridVertexArray.getPrimitiveType() == sf::PrimitiveType::Triangles
        && tileGridVertexArray.getVertexCount() == 24U
        && matchesVertex(
            tileGridVertexArray[0],
            overlayRectangles[0].position.x,
            overlayRectangles[0].position.y,
            gridColor)
        && matchesVertex(
            tileGridVertexArray[5],
            overlayRectangles[0].position.x + overlayRectangles[0].size.width,
            overlayRectangles[0].position.y,
            gridColor)
        && matchesVertex(
            tileGridVertexArray[18],
            overlayRectangles[3].position.x,
            overlayRectangles[3].position.y,
            gridColor)
        && matchesVertex(
            tileGridVertexArray[23],
            overlayRectangles[3].position.x + overlayRectangles[3].size.width,
            overlayRectangles[3].position.y,
            gridColor);
}

bool verifyTileGridBatchUsesBoundsDerivedStrips()
{
    const sf::Color gridColor(255, 255, 255, 96);
    const std::vector<rpg::OverworldRenderTile> visibleTiles{
        makeVisibleTile(2, 3, rpg::TileType::Forest),
        makeVisibleTile(3, 3, rpg::TileType::Forest),
        makeVisibleTile(2, 4, rpg::TileType::Forest),
        makeVisibleTile(3, 4, rpg::TileType::Forest)};
    const sf::VertexArray tileGridVertexArray = rpg::detail::buildTileGridVertexArray(visibleTiles, 1.0F, gridColor);

    return tileGridVertexArray.getPrimitiveType() == sf::PrimitiveType::Triangles
        && tileGridVertexArray.getVertexCount() == 36U
        && matchesVertex(tileGridVertexArray[0], 32.0F, 48.0F, gridColor)
        && matchesVertex(tileGridVertexArray[5], 64.0F, 48.0F, gridColor)
        && matchesVertex(tileGridVertexArray[6], 32.0F, 63.0F, gridColor)
        && matchesVertex(tileGridVertexArray[11], 64.0F, 63.0F, gridColor)
        && matchesVertex(tileGridVertexArray[12], 32.0F, 79.0F, gridColor)
        && matchesVertex(tileGridVertexArray[17], 64.0F, 79.0F, gridColor)
        && matchesVertex(tileGridVertexArray[18], 32.0F, 48.0F, gridColor)
        && matchesVertex(tileGridVertexArray[23], 33.0F, 48.0F, gridColor)
        && matchesVertex(tileGridVertexArray[24], 47.0F, 48.0F, gridColor)
        && matchesVertex(tileGridVertexArray[29], 49.0F, 48.0F, gridColor)
        && matchesVertex(tileGridVertexArray[30], 63.0F, 48.0F, gridColor)
        && matchesVertex(tileGridVertexArray[35], 64.0F, 48.0F, gridColor);
}

bool verifyTileGridBatchTracksBoundsChanges()
{
    const sf::Color gridColor(255, 255, 255, 96);
    const std::vector<rpg::OverworldRenderTile> tighterVisibleTiles{
        makeVisibleTile(2, 3, rpg::TileType::Forest),
        makeVisibleTile(3, 3, rpg::TileType::Forest),
        makeVisibleTile(2, 4, rpg::TileType::Forest),
        makeVisibleTile(3, 4, rpg::TileType::Forest)};
    const std::vector<rpg::OverworldRenderTile> widerVisibleTiles{
        makeVisibleTile(1, 2, rpg::TileType::Forest),
        makeVisibleTile(2, 2, rpg::TileType::Forest),
        makeVisibleTile(3, 2, rpg::TileType::Forest),
        makeVisibleTile(1, 3, rpg::TileType::Forest),
        makeVisibleTile(2, 3, rpg::TileType::Forest),
        makeVisibleTile(3, 3, rpg::TileType::Forest),
        makeVisibleTile(1, 4, rpg::TileType::Forest),
        makeVisibleTile(2, 4, rpg::TileType::Forest),
        makeVisibleTile(3, 4, rpg::TileType::Forest)};
    const sf::VertexArray tighterGridVertexArray = rpg::detail::buildTileGridVertexArray(tighterVisibleTiles, 1.0F, gridColor);
    const sf::VertexArray widerGridVertexArray = rpg::detail::buildTileGridVertexArray(widerVisibleTiles, 1.0F, gridColor);

    return tighterGridVertexArray.getVertexCount() == 36U
        && widerGridVertexArray.getVertexCount() == 48U
        && matchesVertex(tighterGridVertexArray[0], 32.0F, 48.0F, gridColor)
        && matchesVertex(tighterGridVertexArray[5], 64.0F, 48.0F, gridColor)
        && matchesVertex(widerGridVertexArray[0], 16.0F, 32.0F, gridColor)
        && matchesVertex(widerGridVertexArray[5], 64.0F, 32.0F, gridColor)
        && matchesVertex(widerGridVertexArray[18], 16.0F, 79.0F, gridColor)
        && matchesVertex(widerGridVertexArray[23], 64.0F, 79.0F, gridColor);
}

bool verifyDebugOverlayUsesOptimizedGridVertexMetrics()
{
    const sf::Color gridColor(255, 255, 255, 96);
    const std::vector<rpg::OverworldRenderTile> visibleTiles{
        makeVisibleTile(2, 3, rpg::TileType::Forest),
        makeVisibleTile(3, 3, rpg::TileType::Forest),
        makeVisibleTile(2, 4, rpg::TileType::Forest),
        makeVisibleTile(3, 4, rpg::TileType::Forest)};
    const sf::VertexArray tileGridVertexArray = rpg::detail::buildTileGridVertexArray(visibleTiles, 1.0F, gridColor);
    const rpg::OverworldDebugSnapshot debugSnapshot{
        {12, -7},
        150,
        17,
        42,
        9,
        visibleTiles.size(),
        11};
    const rpg::detail::DebugOverlayRenderMetrics enabledRenderMetrics{
        3,
        1536,
        tileGridVertexArray.getVertexCount()};
    const rpg::detail::DebugOverlayRenderMetrics disabledRenderMetrics{
        3,
        1536,
        0};
    const std::string enabledOverlayString = rpg::detail::buildDebugOverlayString(debugSnapshot, enabledRenderMetrics, 144);
    const std::string disabledOverlayString = rpg::detail::buildDebugOverlayString(debugSnapshot, disabledRenderMetrics, 144);

    return enabledOverlayString.find("Grid vertices: 36\n") != std::string::npos
        && disabledOverlayString.find("Grid vertices: 0\n") != std::string::npos;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return EXIT_FAILURE;
    }

    const std::filesystem::path assetRoot = argv[1];

    if (!verifyTerrainBatchUsesExpectedGeometry(assetRoot))
    {
        return EXIT_FAILURE;
    }

    if (!verifyTileGridBatchUsesOverlayRectangles())
    {
        return EXIT_FAILURE;
    }

    if (!verifyTileGridBatchUsesBoundsDerivedStrips())
    {
        return EXIT_FAILURE;
    }

    if (!verifyTileGridBatchTracksBoundsChanges())
    {
        return EXIT_FAILURE;
    }

    if (!verifyDebugOverlayUsesOptimizedGridVertexMetrics())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
