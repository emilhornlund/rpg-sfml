/**
 * @file GameSceneRenderSupport.cpp
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

#include "GameSceneRenderSupport.hpp"

#include "GameRenderBatchSupport.hpp"
#include "GameRenderSupport.hpp"
#include "GameRuntimeSupport.hpp"

namespace rpg::detail
{

OverworldSceneRenderFrame buildOverworldSceneRenderFrame(
    const GameRenderResources& renderResources,
    const OverworldRenderSnapshot& renderSnapshot,
    const float terrainAnimationElapsedSeconds,
    const std::uint32_t worldGenerationSeed,
    const bool renderTileGridOverlay,
    const float tileGridThickness,
    const sf::Color& tileGridColor)
{
    OverworldSceneRenderFrame renderFrame(renderResources.vegetationTileset, renderResources.playerSpritesheet);
    renderFrame.worldTileSize = renderSnapshot.visibleTiles.empty() ? 16.0F : renderSnapshot.visibleTiles.front().size.width;
    renderFrame.vegetationTilesetMetadata = &renderResources.vegetationTilesetMetadata;
    renderFrame.terrainVertexArray = buildTerrainVertexArray(
        renderResources.terrainTilesetMetadata,
        renderSnapshot,
        terrainAnimationElapsedSeconds,
        worldGenerationSeed);
    renderFrame.terrainRenderStates.texture = &renderResources.terrainTileset;

    if (renderTileGridOverlay)
    {
        renderFrame.tileGridVertexArray =
            buildTileGridVertexArray(renderSnapshot.visibleTiles, tileGridThickness, tileGridColor);
    }

    return renderFrame;
}

void renderOverworldScene(
    sf::RenderTarget& renderTarget,
    const sf::View& view,
    OverworldSceneRenderFrame& renderFrame,
    const OverworldRenderSnapshot& renderSnapshot,
    const OverworldRenderPlan& renderPlan)
{
    executeViewFramedRender(
        [&]()
        {
            renderTarget.setView(view);
        },
        [&]()
        {
            executeOverworldRenderPasses(
                [&]()
                {
                    renderTarget.draw(renderFrame.terrainVertexArray, renderFrame.terrainRenderStates);
                },
                [&]()
                {
                    for (const OverworldRenderQueueEntry& entry : renderPlan.renderQueue)
                    {
                        if (entry.kind == OverworldRenderQueueEntryKind::GeneratedContent)
                        {
                            drawVegetationContent(
                                renderTarget,
                                renderFrame.vegetationSprite,
                                *renderFrame.vegetationTilesetMetadata,
                                renderSnapshot.generatedContent[entry.sourceIndex],
                                renderFrame.worldTileSize);
                            continue;
                        }

                        drawPlayerMarker(
                            renderTarget,
                            renderFrame.playerSprite,
                            renderSnapshot.markers[entry.sourceIndex]);
                    }
                },
                [&]()
                {
                    renderTarget.draw(renderFrame.tileGridVertexArray);
                },
                renderFrame.tileGridVertexArray.getVertexCount() > 0U);
        });
}

} // namespace rpg::detail
