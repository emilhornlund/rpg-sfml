/**
 * @file GameSceneRenderSupportTests.cpp
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

#include "GameResourceBootstrapSupport.hpp"
#include "GameSceneRenderSupport.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <cmath>
#include <filesystem>

namespace
{

[[nodiscard]] sf::VertexArray makeSolidQuadVertexArray(const sf::Color& color)
{
    sf::VertexArray vertexArray(sf::PrimitiveType::Triangles, 6U);
    vertexArray[0] = {{0.0F, 0.0F}, color};
    vertexArray[1] = {{0.0F, 16.0F}, color};
    vertexArray[2] = {{16.0F, 16.0F}, color};
    vertexArray[3] = {{0.0F, 0.0F}, color};
    vertexArray[4] = {{16.0F, 16.0F}, color};
    vertexArray[5] = {{16.0F, 0.0F}, color};
    return vertexArray;
}

[[nodiscard]] sf::Texture makeSolidTexture(const sf::Vector2u size, const sf::Color& color)
{
    const sf::Image image(size, color);
    sf::Texture texture(image);
    texture.setSmooth(false);
    return texture;
}

[[nodiscard]] sf::Color renderCenterPixel(
    rpg::detail::OverworldSceneRenderFrame& renderFrame,
    const rpg::OverworldRenderSnapshot& renderSnapshot,
    const rpg::detail::OverworldRenderPlan& renderPlan)
{
    sf::RenderTexture renderTexture;

    if (!renderTexture.resize({16U, 16U}))
    {
        return sf::Color::Transparent;
    }

    renderTexture.clear(sf::Color::Transparent);
    const sf::View view({8.0F, 8.0F}, {16.0F, 16.0F});
    rpg::detail::renderOverworldScene(renderTexture, view, renderFrame, renderSnapshot, renderPlan);
    renderTexture.display();
    return renderTexture.getTexture().copyToImage().getPixel({8U, 8U});
}

[[nodiscard]] rpg::OverworldRenderSnapshot makeRenderSnapshot()
{
    return {
        {{0.0F, 0.0F}, {320.0F, 180.0F}},
        {{
             {{0, 0}, rpg::TileType::Grass, {16.0F, 16.0F}, {8.0F, 8.0F}, {8.0F, 8.0F}},
             {{1, 0}, rpg::TileType::Grass, {16.0F, 16.0F}, {8.0F, 8.0F}, {24.0F, 8.0F}},
         }},
        {{
             {{0, 0}, rpg::TileType::Grass, {16.0F, 16.0F}, {8.0F, 8.0F}, {8.0F, 8.0F}},
         }},
        {{
             {11, rpg::ContentType::Shrub, "bush_small_1", {0, 0}, {32.0F, 32.0F}, {8.0F, 24.0F}, {8.0F, 24.0F}, {18}, 24.0F},
         }},
        {{
             {{48.0F, 48.0F}, {24.0F, 32.0F}, {24.0F, 40.0F}, rpg::OverworldRenderMarkerAppearance::Player, rpg::PlayerFacingDirection::Down, 1, 40.0F},
         }}};
}

[[nodiscard]] bool verifySceneRenderFramePreparation(const std::filesystem::path& assetRoot)
{
    const rpg::detail::GameRenderResources renderResources = rpg::detail::loadGameRenderResources(assetRoot);
    const rpg::OverworldRenderSnapshot renderSnapshot = makeRenderSnapshot();
    const rpg::detail::OverworldSceneRenderFrame renderFrame = rpg::detail::buildOverworldSceneRenderFrame(
        renderResources,
        renderSnapshot,
        0.5F,
        1337U,
        true,
        1.0F,
        sf::Color(255, 255, 255, 96));

    return renderFrame.worldTileSize == 16.0F
        && renderFrame.vegetationTilesetMetadata == &renderResources.vegetationTilesetMetadata
        && renderFrame.terrainRenderStates.texture == &renderResources.terrainTileset
        && renderFrame.roadOverlayRenderStates.texture == &renderResources.groundOverlayTileset
        && renderFrame.terrainVertexArray.getVertexCount() == renderSnapshot.visibleTiles.size() * 6U
        && renderFrame.roadOverlayVertexArray.getVertexCount() == renderSnapshot.visibleRoadOverlays.size() * 6U
        && renderFrame.tileGridVertexArray.getVertexCount() > 0U
        && renderFrame.vegetationSprite.getTexture().getSize() == renderResources.vegetationTileset.getSize()
        && renderFrame.playerSprite.getTexture().getSize() == renderResources.playerSpritesheet.getSize();
}

[[nodiscard]] bool verifySceneRenderFrameSkipsTileGridWhenDisabled(const std::filesystem::path& assetRoot)
{
    const rpg::detail::GameRenderResources renderResources = rpg::detail::loadGameRenderResources(assetRoot);
    const rpg::detail::OverworldSceneRenderFrame renderFrame = rpg::detail::buildOverworldSceneRenderFrame(
        renderResources,
        makeRenderSnapshot(),
        0.5F,
        1337U,
        false,
        1.0F,
        sf::Color(255, 255, 255, 96));

    return renderFrame.tileGridVertexArray.getVertexCount() == 0U;
}

[[nodiscard]] bool verifySceneRenderPassOrdering(const std::filesystem::path& assetRoot)
{
    const rpg::detail::VegetationTilesetMetadata vegetationMetadata = rpg::detail::loadVegetationTilesetMetadata(assetRoot);
    const sf::Texture vegetationTileset = makeSolidTexture({512U, 512U}, sf::Color::Red);
    const sf::Texture playerSpritesheet = makeSolidTexture({144U, 192U}, sf::Color::Blue);
    rpg::detail::OverworldSceneRenderFrame renderFrame(vegetationTileset, playerSpritesheet);
    renderFrame.worldTileSize = 16.0F;
    renderFrame.vegetationTilesetMetadata = &vegetationMetadata;
    renderFrame.terrainVertexArray = makeSolidQuadVertexArray(sf::Color::Green);
    renderFrame.roadOverlayVertexArray = makeSolidQuadVertexArray(sf::Color(120, 72, 24));

    rpg::OverworldRenderSnapshot renderSnapshot;
    renderSnapshot.cameraFrame = {{8.0F, 8.0F}, {16.0F, 16.0F}};
    renderSnapshot.generatedContent = {{
        11,
        rpg::ContentType::Shrub,
        "bush_small_1",
        {0, 0},
        {16.0F, 16.0F},
        {8.0F, 8.0F},
        {8.0F, 8.0F},
        {18},
        8.0F,
    }};
    renderSnapshot.markers = {{
        {16.0F, 16.0F},
        {8.0F, 8.0F},
        {8.0F, 8.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Down,
        1,
        8.0F,
    }};

    const rpg::detail::OverworldRenderPlan terrainOnlyPlan{};
    const rpg::detail::OverworldRenderPlan contentPlan{
        {{{rpg::detail::OverworldRenderQueueEntryKind::GeneratedContent, {8.0F, 0U, 11U}, 0U}}},
        std::nullopt,
        {},
        {},
        {}};
    const rpg::detail::OverworldRenderPlan playerPlan{
        {{{rpg::detail::OverworldRenderQueueEntryKind::PlayerMarker, {8.0F, 1U, 1U}, 0U}}},
        0U,
        {},
        {},
        {}};

    const sf::Color roadPixel = renderCenterPixel(renderFrame, renderSnapshot, terrainOnlyPlan);
    const sf::Color vegetationPixel = renderCenterPixel(renderFrame, renderSnapshot, contentPlan);
    const sf::Color playerPixel = renderCenterPixel(renderFrame, renderSnapshot, playerPlan);

    return roadPixel == sf::Color(120, 72, 24)
        && vegetationPixel == sf::Color::Red
        && playerPixel == sf::Color::Blue;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 1;
    }

    const std::filesystem::path assetRoot = argv[1];

    if (!verifySceneRenderFramePreparation(assetRoot))
    {
        return 1;
    }

    if (!verifySceneRenderFrameSkipsTileGridWhenDisabled(assetRoot))
    {
        return 1;
    }

    if (!verifySceneRenderPassOrdering(assetRoot))
    {
        return 1;
    }

    return 0;
}
