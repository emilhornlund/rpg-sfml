/**
 * @file GameOcclusionCompositeSupport.hpp
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

#ifndef RPG_MAIN_GAME_OCCLUSION_COMPOSITE_SUPPORT_HPP
#define RPG_MAIN_GAME_OCCLUSION_COMPOSITE_SUPPORT_HPP

#include "GameAssetSupport.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/View.hpp>

#include <cstddef>
#include <vector>

namespace rpg::detail
{

struct PlayerOcclusionCompositeRuntime
{
    sf::RenderTexture playerMaskTexture;
    sf::RenderTexture occluderMaskTexture;
};

[[nodiscard]] sf::Vector2u makeOcclusionRenderSurfaceSize(sf::Vector2u outputSize) noexcept;

void ensureRenderTextureSize(sf::RenderTexture& renderTexture, sf::Vector2u size);

void configureOcclusionCompositeSprite(sf::Sprite& sprite, sf::Vector2u outputSize);

void drawPlayerOcclusionComposite(
    sf::RenderTarget& outputTarget,
    PlayerOcclusionCompositeRuntime& runtime,
    sf::Shader& playerOcclusionShader,
    const sf::View& worldView,
    const sf::View& overlayView,
    sf::Sprite& playerSprite,
    sf::Sprite& vegetationSprite,
    const VegetationTilesetMetadata& vegetationTilesetMetadata,
    const std::vector<OverworldRenderContent>& generatedContent,
    const OverworldRenderMarker& playerMarker,
    const std::vector<std::size_t>& overlapQualifiedOcclusionCandidateIndices,
    float worldTileSize);

} // namespace rpg::detail

#endif // RPG_MAIN_GAME_OCCLUSION_COMPOSITE_SUPPORT_HPP
