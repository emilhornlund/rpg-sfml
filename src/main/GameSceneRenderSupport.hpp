/**
 * @file GameSceneRenderSupport.hpp
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

#ifndef RPG_MAIN_GAME_SCENE_RENDER_SUPPORT_HPP
#define RPG_MAIN_GAME_SCENE_RENDER_SUPPORT_HPP

#include "GameRenderPlanSupport.hpp"
#include "GameResourceBootstrapSupport.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/View.hpp>

#include <cstdint>

namespace rpg::detail
{

struct OverworldSceneRenderFrame
{
    OverworldSceneRenderFrame(
        const sf::Texture& vegetationTileset,
        const sf::Texture& playerSpritesheet)
        : vegetationSprite(vegetationTileset)
        , playerSprite(playerSpritesheet)
    {
    }

    float worldTileSize = 16.0F;
    const VegetationTilesetMetadata* vegetationTilesetMetadata = nullptr;
    sf::VertexArray terrainVertexArray{sf::PrimitiveType::Triangles};
    sf::RenderStates terrainRenderStates;
    sf::VertexArray roadOverlayVertexArray{sf::PrimitiveType::Triangles};
    sf::RenderStates roadOverlayRenderStates;
    sf::Sprite vegetationSprite;
    sf::Sprite playerSprite;
    sf::VertexArray tileGridVertexArray{sf::PrimitiveType::Triangles};
};

[[nodiscard]] OverworldSceneRenderFrame buildOverworldSceneRenderFrame(
    const GameRenderResources& renderResources,
    const OverworldRenderSnapshot& renderSnapshot,
    float terrainAnimationElapsedSeconds,
    std::uint32_t worldGenerationSeed,
    bool renderTileGridOverlay,
    float tileGridThickness,
    const sf::Color& tileGridColor);

void renderOverworldScene(
    sf::RenderTarget& renderTarget,
    const sf::View& view,
    OverworldSceneRenderFrame& renderFrame,
    const OverworldRenderSnapshot& renderSnapshot,
    const OverworldRenderPlan& renderPlan);

} // namespace rpg::detail

#endif // RPG_MAIN_GAME_SCENE_RENDER_SUPPORT_HPP
