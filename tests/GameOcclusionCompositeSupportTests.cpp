/**
 * @file GameOcclusionCompositeSupportTests.cpp
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

#include <SFML/Graphics/RenderTexture.hpp>

#include <cmath>

namespace
{

[[nodiscard]] bool verifyOcclusionRenderSurfaceSizing()
{
    return rpg::detail::makeOcclusionRenderSurfaceSize({1280U, 720U}) == sf::Vector2u({640U, 360U})
        && rpg::detail::makeOcclusionRenderSurfaceSize({1U, 1U}) == sf::Vector2u({1U, 1U})
        && rpg::detail::makeOcclusionRenderSurfaceSize({3U, 5U}) == sf::Vector2u({2U, 3U});
}

[[nodiscard]] bool verifyRenderTextureSizing()
{
    sf::RenderTexture renderTexture;
    rpg::detail::ensureRenderTextureSize(renderTexture, {64U, 32U});

    if (renderTexture.getSize() != sf::Vector2u({64U, 32U}) || renderTexture.isSmooth())
    {
        return false;
    }

    rpg::detail::ensureRenderTextureSize(renderTexture, {64U, 32U});
    return renderTexture.getSize() == sf::Vector2u({64U, 32U});
}

[[nodiscard]] bool verifyOcclusionCompositeSpriteConfiguration()
{
    sf::Texture texture({64U, 32U});
    sf::Sprite sprite(texture);
    rpg::detail::configureOcclusionCompositeSprite(sprite, {128U, 96U});

    return std::fabs(sprite.getPosition().x) < 0.0001F
        && std::fabs(sprite.getPosition().y) < 0.0001F
        && std::fabs(sprite.getScale().x - 2.0F) < 0.0001F
        && std::fabs(sprite.getScale().y - 3.0F) < 0.0001F;
}

} // namespace

int main()
{
    if (!verifyOcclusionRenderSurfaceSizing())
    {
        return 1;
    }

    if (!verifyRenderTextureSizing())
    {
        return 1;
    }

    if (!verifyOcclusionCompositeSpriteConfiguration())
    {
        return 1;
    }

    return 0;
}
