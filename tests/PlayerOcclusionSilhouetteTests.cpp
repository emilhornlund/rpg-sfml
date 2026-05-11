/**
 * @file PlayerOcclusionSilhouetteTests.cpp
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

#include "PlayerOcclusionSilhouetteSupport.hpp"
#include "GameRuntimeSupport.hpp"

#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <cmath>
#include <iostream>

namespace
{

bool verifyPlayerOcclusionShaderCompositesOnlyOverlap()
{
    sf::RenderTexture resultTexture;
    const sf::Vector2u textureSize{8U, 8U};

    if (!resultTexture.resize(textureSize))
    {
        return false;
    }

    sf::Image playerImage(textureSize, sf::Color::Transparent);
    sf::Image occluderImage(textureSize, sf::Color::Transparent);

    for (unsigned int y = 1U; y < 5U; ++y)
    {
        for (unsigned int x = 1U; x < 5U; ++x)
        {
            playerImage.setPixel({x, y}, sf::Color::White);
        }
    }

    for (unsigned int y = 1U; y < 5U; ++y)
    {
        for (unsigned int x = 3U; x < 6U; ++x)
        {
            occluderImage.setPixel({x, y}, sf::Color::White);
        }
    }

    const sf::Texture playerTexture(playerImage);
    const sf::Texture occluderTexture(occluderImage);
    resultTexture.clear(sf::Color::Transparent);

    sf::Shader shader = rpg::detail::loadPlayerOcclusionShader();
    shader.setUniform("currentTexture", sf::Shader::CurrentTexture);
    shader.setUniform("occluderMask", occluderTexture);
    shader.setUniform("silhouetteColor", sf::Glsl::Vec4(rpg::detail::kPlayerOcclusionSilhouetteColor));

    sf::Sprite playerSprite(playerTexture);
    resultTexture.draw(playerSprite, &shader);
    resultTexture.display();

    const sf::Image resultImage = resultTexture.getTexture().copyToImage();
    const sf::Color clearPixel = resultImage.getPixel({1U, 1U});
    const sf::Color overlapPixel = resultImage.getPixel({3U, 2U});
    const sf::Color outsidePlayerPixel = resultImage.getPixel({7U, 7U});
    const std::uint8_t expectedRed =
        static_cast<std::uint8_t>((static_cast<unsigned int>(rpg::detail::kPlayerOcclusionSilhouetteColor.r)
                                   * static_cast<unsigned int>(rpg::detail::kPlayerOcclusionSilhouetteColor.a)
                                   + 127U)
                                  / 255U);
    const std::uint8_t expectedGreen =
        static_cast<std::uint8_t>((static_cast<unsigned int>(rpg::detail::kPlayerOcclusionSilhouetteColor.g)
                                   * static_cast<unsigned int>(rpg::detail::kPlayerOcclusionSilhouetteColor.a)
                                   + 127U)
                                  / 255U);
    const std::uint8_t expectedBlue =
        static_cast<std::uint8_t>((static_cast<unsigned int>(rpg::detail::kPlayerOcclusionSilhouetteColor.b)
                                   * static_cast<unsigned int>(rpg::detail::kPlayerOcclusionSilhouetteColor.a)
                                   + 127U)
                                  / 255U);
    const bool matches = clearPixel.a == 0U
        && outsidePlayerPixel.a == 0U
        && overlapPixel.a == rpg::detail::kPlayerOcclusionSilhouetteColor.a
        && overlapPixel.r == expectedRed
        && overlapPixel.g == expectedGreen
        && overlapPixel.b == expectedBlue;

    if (!matches)
    {
        std::cerr
            << "clear=("
            << static_cast<int>(clearPixel.r) << ','
            << static_cast<int>(clearPixel.g) << ','
            << static_cast<int>(clearPixel.b) << ','
            << static_cast<int>(clearPixel.a) << ") overlap=("
            << static_cast<int>(overlapPixel.r) << ','
            << static_cast<int>(overlapPixel.g) << ','
            << static_cast<int>(overlapPixel.b) << ','
            << static_cast<int>(overlapPixel.a) << ") outside=("
            << static_cast<int>(outsidePlayerPixel.r) << ','
            << static_cast<int>(outsidePlayerPixel.g) << ','
            << static_cast<int>(outsidePlayerPixel.b) << ','
            << static_cast<int>(outsidePlayerPixel.a) << ")\n";
    }

    return matches;
}

bool verifyResizedScreenSpaceCompositeKeepsSilhouetteCentered()
{
    constexpr sf::Vector2u initialSize{640U, 360U};
    constexpr sf::Vector2u resizedSize{960U, 540U};
    constexpr rpg::ViewFrame worldFrame{{160.0F, 90.0F}, {320.0F, 180.0F}};
    constexpr sf::Vector2f playerCenter{160.0F, 90.0F};

    sf::RenderTexture playerMaskTexture;
    sf::RenderTexture occluderMaskTexture;
    sf::RenderTexture compositeTexture;

    if (!playerMaskTexture.resize(resizedSize)
        || !occluderMaskTexture.resize(resizedSize)
        || !compositeTexture.resize(resizedSize))
    {
        return false;
    }

    sf::View worldView;
    worldView.setCenter({worldFrame.center.x, worldFrame.center.y});
    worldView.setSize({worldFrame.size.width, worldFrame.size.height});

    sf::RectangleShape playerShape({48.0F, 48.0F});
    playerShape.setOrigin({24.0F, 24.0F});
    playerShape.setPosition(playerCenter);
    playerShape.setFillColor(sf::Color::White);

    sf::RectangleShape occluderShape({48.0F, 48.0F});
    occluderShape.setOrigin({24.0F, 24.0F});
    occluderShape.setPosition(playerCenter);
    occluderShape.setFillColor(sf::Color::White);

    playerMaskTexture.setView(worldView);
    playerMaskTexture.clear(sf::Color::Transparent);
    playerMaskTexture.draw(playerShape);
    playerMaskTexture.display();

    occluderMaskTexture.setView(worldView);
    occluderMaskTexture.clear(sf::Color::Transparent);
    occluderMaskTexture.draw(occluderShape);
    occluderMaskTexture.display();

    sf::Shader shader = rpg::detail::loadPlayerOcclusionShader();
    shader.setUniform("currentTexture", sf::Shader::CurrentTexture);
    shader.setUniform("occluderMask", occluderMaskTexture.getTexture());
    shader.setUniform("silhouetteColor", sf::Glsl::Vec4(rpg::detail::kPlayerOcclusionSilhouetteColor));

    sf::Sprite compositeSprite(playerMaskTexture.getTexture());
    sf::View overlayView;
    const rpg::ViewFrame initialOverlayFrame =
        rpg::detail::makeScreenSpaceViewFrame(initialSize.x, initialSize.y);
    overlayView.setCenter({initialOverlayFrame.center.x, initialOverlayFrame.center.y});
    overlayView.setSize({initialOverlayFrame.size.width, initialOverlayFrame.size.height});
    const rpg::ViewFrame resizedOverlayFrame =
        rpg::detail::makeScreenSpaceViewFrame(resizedSize.x, resizedSize.y);
    overlayView.setCenter({resizedOverlayFrame.center.x, resizedOverlayFrame.center.y});
    overlayView.setSize({resizedOverlayFrame.size.width, resizedOverlayFrame.size.height});

    compositeTexture.setView(overlayView);
    compositeTexture.clear(sf::Color::Transparent);
    compositeTexture.draw(compositeSprite, &shader);
    compositeTexture.display();

    const sf::Image compositeImage = compositeTexture.getTexture().copyToImage();
    bool foundOpaquePixel = false;
    unsigned int minX = resizedSize.x;
    unsigned int maxX = 0U;
    unsigned int minY = resizedSize.y;
    unsigned int maxY = 0U;

    for (unsigned int y = 0U; y < resizedSize.y; ++y)
    {
        for (unsigned int x = 0U; x < resizedSize.x; ++x)
        {
            if (compositeImage.getPixel({x, y}).a == 0U)
            {
                continue;
            }

            foundOpaquePixel = true;
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }
    }

    if (!foundOpaquePixel)
    {
        return false;
    }

    const float centerX = (static_cast<float>(minX) + static_cast<float>(maxX)) * 0.5F;
    const float centerY = (static_cast<float>(minY) + static_cast<float>(maxY)) * 0.5F;

    return std::fabs(centerX - static_cast<float>(resizedSize.x) * 0.5F) <= 1.0F
        && std::fabs(centerY - static_cast<float>(resizedSize.y) * 0.5F) <= 1.0F;
}

} // namespace

int main()
{
    if (!verifyPlayerOcclusionShaderCompositesOnlyOverlap())
    {
        return 1;
    }

    if (!verifyResizedScreenSpaceCompositeKeepsSilhouetteCentered())
    {
        return 1;
    }

    return 0;
}
