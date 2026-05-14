/**
 * @file GameRenderSupportTests.cpp
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

#include "GameRenderSupport.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <cmath>
#include <cstdlib>

namespace
{

[[nodiscard]] bool areClose(const float lhs, const float rhs) noexcept
{
    return std::fabs(lhs - rhs) < 0.0001F;
}

bool verifyPlayerSpritesheetRectSelection()
{
    const rpg::OverworldRenderMarker downMarker{
        {48.0F, 48.0F},
        {24.0F, 32.0F},
        {72.0F, 136.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Down,
        -4,
        136.0F};
    const rpg::OverworldRenderMarker rightMarker{
        {48.0F, 48.0F},
        {24.0F, 32.0F},
        {72.0F, 136.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Right,
        1,
        136.0F};
    const rpg::OverworldRenderMarker upMarker{
        {48.0F, 48.0F},
        {24.0F, 32.0F},
        {72.0F, 136.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Up,
        8,
        136.0F};

    return rpg::detail::getPlayerSpritesheetRect(downMarker) == sf::IntRect({0, 0}, {48, 48})
        && rpg::detail::getPlayerSpritesheetRect(rightMarker) == sf::IntRect({48, 96}, {48, 48})
        && rpg::detail::getPlayerSpritesheetRect(upMarker) == sf::IntRect({96, 144}, {48, 48});
}

bool verifyPlayerMarkerSpriteConfiguration()
{
    const rpg::OverworldRenderMarker renderMarker{
        {96.0F, 144.0F},
        {30.0F, 90.0F},
        {144.0F, 208.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Left,
        2,
        208.0F};
    sf::Texture texture({48U, 48U});
    sf::Sprite sprite(texture);

    rpg::detail::configurePlayerMarkerSprite(sprite, renderMarker);

    return sprite.getTextureRect() == sf::IntRect({96, 48}, {48, 48})
        && areClose(sprite.getScale().x, 2.0F)
        && areClose(sprite.getScale().y, 3.0F)
        && areClose(sprite.getOrigin().x, 15.0F)
        && areClose(sprite.getOrigin().y, 30.0F)
        && areClose(sprite.getPosition().x, renderMarker.position.x)
        && areClose(sprite.getPosition().y, renderMarker.position.y);
}

bool verifyViewFrameApplication()
{
    const rpg::ViewFrame frame{
        {103.5F, 207.25F},
        {320.0F, 180.0F}};
    sf::View view;

    rpg::detail::applyViewFrame(view, frame);

    return areClose(view.getCenter().x, frame.center.x)
        && areClose(view.getCenter().y, frame.center.y)
        && areClose(view.getSize().x, frame.size.width)
        && areClose(view.getSize().y, frame.size.height);
}

} // namespace

int main()
{
    if (!verifyPlayerSpritesheetRectSelection())
    {
        return EXIT_FAILURE;
    }

    if (!verifyPlayerMarkerSpriteConfiguration())
    {
        return EXIT_FAILURE;
    }

    if (!verifyViewFrameApplication())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
