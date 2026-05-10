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

#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

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

} // namespace

int main()
{
    if (!verifyPlayerOcclusionShaderCompositesOnlyOverlap())
    {
        return 1;
    }

    return 0;
}
