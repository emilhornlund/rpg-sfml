/**
 * @file GameRenderSupport.cpp
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

#include <algorithm>

namespace rpg::detail
{

namespace
{

constexpr int kPlayerSpritesheetCellSize = 48;
constexpr int kVegetationTilesetCellSize = 16;

[[nodiscard]] int getPlayerSpritesheetRow(const PlayerFacingDirection facingDirection) noexcept
{
    switch (facingDirection)
    {
    case PlayerFacingDirection::Down:
        return 0;
    case PlayerFacingDirection::Left:
        return 1;
    case PlayerFacingDirection::Right:
        return 2;
    case PlayerFacingDirection::Up:
        return 3;
    }

    return 0;
}

} // namespace

[[nodiscard]] sf::IntRect getVegetationTilesetRect(const VegetationAtlasCell& cell) noexcept
{
    return {
        {cell.tileX * kVegetationTilesetCellSize, cell.tileY * kVegetationTilesetCellSize},
        {kVegetationTilesetCellSize, kVegetationTilesetCellSize}};
}

sf::IntRect getPlayerSpritesheetRect(const OverworldRenderMarker& renderMarker) noexcept
{
    const int frameIndex = std::clamp(renderMarker.animationFrameIndex, 0, 2);
    const int rowIndex = getPlayerSpritesheetRow(renderMarker.facingDirection);
    return {
        {frameIndex * kPlayerSpritesheetCellSize, rowIndex * kPlayerSpritesheetCellSize},
        {kPlayerSpritesheetCellSize, kPlayerSpritesheetCellSize}};
}

void configurePlayerMarkerSprite(sf::Sprite& sprite, const OverworldRenderMarker& renderMarker) noexcept
{
    const float scaleX = renderMarker.size.width / static_cast<float>(kPlayerSpritesheetCellSize);
    const float scaleY = renderMarker.size.height / static_cast<float>(kPlayerSpritesheetCellSize);
    sprite.setTextureRect(getPlayerSpritesheetRect(renderMarker));
    sprite.setScale({scaleX, scaleY});
    sprite.setOrigin({renderMarker.origin.x / scaleX, renderMarker.origin.y / scaleY});
    sprite.setPosition({renderMarker.position.x, renderMarker.position.y});
}

void drawPlayerMarker(sf::RenderTarget& target, sf::Sprite& sprite, const OverworldRenderMarker& renderMarker)
{
    configurePlayerMarkerSprite(sprite, renderMarker);
    target.draw(sprite);
}

void drawVegetationContent(
    sf::RenderTarget& target,
    sf::Sprite& vegetationSprite,
    const VegetationTilesetMetadata& vegetationTilesetMetadata,
    const OverworldRenderContent& renderContent,
    const float worldTileSize)
{
    const VegetationPrototype& prototype = vegetationTilesetMetadata.getPrototypeById(renderContent.prototypeId);

    for (const VegetationAtlasPart& part : prototype.parts)
    {
        const float scaleX = worldTileSize / static_cast<float>(kVegetationTilesetCellSize);
        const float scaleY = worldTileSize / static_cast<float>(kVegetationTilesetCellSize);
        vegetationSprite.setTextureRect(getVegetationTilesetRect(part.cell));
        vegetationSprite.setScale({scaleX, scaleY});
        vegetationSprite.setOrigin({8.0F, 8.0F});
        vegetationSprite.setPosition({
            renderContent.position.x + static_cast<float>(part.offsetX) * worldTileSize,
            renderContent.position.y + static_cast<float>(part.offsetY) * worldTileSize});
        target.draw(vegetationSprite);
    }
}

void applyViewFrame(sf::View& view, const ViewFrame& frame) noexcept
{
    view.setCenter({frame.center.x, frame.center.y});
    view.setSize({frame.size.width, frame.size.height});
}

} // namespace rpg::detail
