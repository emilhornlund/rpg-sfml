/**
 * @file GameOcclusionCompositeSupport.cpp
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

#include "GameOcclusionCompositeSupport.hpp"

#include "GameRenderSupport.hpp"
#include "PlayerOcclusionSilhouetteSupport.hpp"

#include <algorithm>
#include <stdexcept>

namespace rpg::detail
{

namespace
{

constexpr unsigned int kOcclusionRenderSurfaceScaleDivisor = 2U;

} // namespace

sf::Vector2u makeOcclusionRenderSurfaceSize(const sf::Vector2u outputSize) noexcept
{
    return {
        std::max(1U, (outputSize.x + kOcclusionRenderSurfaceScaleDivisor - 1U) / kOcclusionRenderSurfaceScaleDivisor),
        std::max(1U, (outputSize.y + kOcclusionRenderSurfaceScaleDivisor - 1U) / kOcclusionRenderSurfaceScaleDivisor)};
}

void ensureRenderTextureSize(sf::RenderTexture& renderTexture, const sf::Vector2u size)
{
    if (size.x == 0U || size.y == 0U)
    {
        throw std::runtime_error("Failed to initialize player occlusion silhouette render target: window size is invalid.");
    }

    if (const sf::Vector2u currentSize = renderTexture.getSize(); currentSize.x != size.x || currentSize.y != size.y)
    {
        if (!renderTexture.resize(size))
        {
            throw std::runtime_error("Failed to resize player occlusion silhouette render target.");
        }

        renderTexture.setSmooth(false);
    }
}

void configureOcclusionCompositeSprite(sf::Sprite& sprite, const sf::Vector2u outputSize)
{
    const sf::Vector2u textureSize = sprite.getTexture().getSize();

    if (textureSize.x == 0U || textureSize.y == 0U)
    {
        throw std::runtime_error("Failed to configure player occlusion silhouette sprite: render target size is invalid.");
    }

    sprite.setPosition({0.0F, 0.0F});
    sprite.setScale({
        static_cast<float>(outputSize.x) / static_cast<float>(textureSize.x),
        static_cast<float>(outputSize.y) / static_cast<float>(textureSize.y)});
}

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
    const float worldTileSize)
{
    if (overlapQualifiedOcclusionCandidateIndices.empty())
    {
        return;
    }

    const sf::Vector2u outputSize = outputTarget.getSize();
    const sf::Vector2u occlusionRenderSurfaceSize = makeOcclusionRenderSurfaceSize(outputSize);
    ensureRenderTextureSize(runtime.playerMaskTexture, occlusionRenderSurfaceSize);
    ensureRenderTextureSize(runtime.occluderMaskTexture, occlusionRenderSurfaceSize);

    runtime.playerMaskTexture.setView(worldView);
    runtime.occluderMaskTexture.setView(worldView);
    runtime.playerMaskTexture.clear(sf::Color::Transparent);
    runtime.occluderMaskTexture.clear(sf::Color::Transparent);

    drawPlayerMarker(runtime.playerMaskTexture, playerSprite, playerMarker);

    for (const std::size_t index : overlapQualifiedOcclusionCandidateIndices)
    {
        drawVegetationContent(
            runtime.occluderMaskTexture,
            vegetationSprite,
            vegetationTilesetMetadata,
            generatedContent[index],
            worldTileSize);
    }

    runtime.playerMaskTexture.display();
    runtime.occluderMaskTexture.display();

    sf::Sprite playerOcclusionSprite(runtime.playerMaskTexture.getTexture());
    configureOcclusionCompositeSprite(playerOcclusionSprite, outputSize);
    playerOcclusionShader.setUniform("currentTexture", sf::Shader::CurrentTexture);
    playerOcclusionShader.setUniform("occluderMask", runtime.occluderMaskTexture.getTexture());
    playerOcclusionShader.setUniform("silhouetteColor", sf::Glsl::Vec4(kPlayerOcclusionSilhouetteColor));

    outputTarget.setView(overlayView);
    outputTarget.draw(playerOcclusionSprite, &playerOcclusionShader);
}

} // namespace rpg::detail
