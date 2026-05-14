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

#include <filesystem>

namespace
{

[[nodiscard]] rpg::OverworldRenderSnapshot makeRenderSnapshot()
{
    return {
        {{0.0F, 0.0F}, {320.0F, 180.0F}},
        {{
             {{0, 0}, rpg::TileType::Grass, {16.0F, 16.0F}, {8.0F, 8.0F}, {8.0F, 8.0F}},
             {{1, 0}, rpg::TileType::Grass, {16.0F, 16.0F}, {8.0F, 8.0F}, {24.0F, 8.0F}},
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
        && renderFrame.terrainVertexArray.getVertexCount() == renderSnapshot.visibleTiles.size() * 6U
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

    return 0;
}
