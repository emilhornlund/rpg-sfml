/**
 * @file RoadOverlayTilesetSupport.cpp
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

#include "RoadOverlayTilesetSupport.hpp"

#include <array>
#include <stdexcept>

namespace rpg
{
namespace detail
{

namespace
{

[[nodiscard]] TileType parseSurfaceType(const std::string& value)
{
    if (value == "grass")
    {
        return TileType::Grass;
    }

    if (value == "sand")
    {
        return TileType::Sand;
    }

    if (value == "forest")
    {
        return TileType::Forest;
    }

    throw std::runtime_error("Unknown road overlay surface type in tileset catalog: " + value);
}

[[nodiscard]] RoadOverlayAutotileRole parseRoadOverlayRole(const std::string& value)
{
    if (value == "outer_top_left")
    {
        return RoadOverlayAutotileRole::OuterTopLeft;
    }

    if (value == "outer_top_right")
    {
        return RoadOverlayAutotileRole::OuterTopRight;
    }

    if (value == "outer_bottom_left")
    {
        return RoadOverlayAutotileRole::OuterBottomLeft;
    }

    if (value == "outer_bottom_right")
    {
        return RoadOverlayAutotileRole::OuterBottomRight;
    }

    if (value == "top_left")
    {
        return RoadOverlayAutotileRole::TopLeft;
    }

    if (value == "top")
    {
        return RoadOverlayAutotileRole::Top;
    }

    if (value == "top_right")
    {
        return RoadOverlayAutotileRole::TopRight;
    }

    if (value == "left")
    {
        return RoadOverlayAutotileRole::Left;
    }

    if (value == "right")
    {
        return RoadOverlayAutotileRole::Right;
    }

    if (value == "bottom_left")
    {
        return RoadOverlayAutotileRole::BottomLeft;
    }

    if (value == "bottom")
    {
        return RoadOverlayAutotileRole::Bottom;
    }

    if (value == "bottom_right")
    {
        return RoadOverlayAutotileRole::BottomRight;
    }

    throw std::runtime_error("Unknown road overlay autotile role in tileset catalog: " + value);
}

void validateMetadata(const RoadOverlayTilesetMetadata& metadata)
{
    if (metadata.getBaseVariantCount() == 0)
    {
        throw std::runtime_error("Missing road overlay base variants in overlay tileset catalog");
    }

    constexpr std::array<RoadOverlayAutotileRole, 12> kRequiredRoles{{
        RoadOverlayAutotileRole::OuterTopLeft,
        RoadOverlayAutotileRole::OuterTopRight,
        RoadOverlayAutotileRole::OuterBottomLeft,
        RoadOverlayAutotileRole::OuterBottomRight,
        RoadOverlayAutotileRole::TopLeft,
        RoadOverlayAutotileRole::Top,
        RoadOverlayAutotileRole::TopRight,
        RoadOverlayAutotileRole::Left,
        RoadOverlayAutotileRole::Right,
        RoadOverlayAutotileRole::BottomLeft,
        RoadOverlayAutotileRole::Bottom,
        RoadOverlayAutotileRole::BottomRight,
    }};

    for (const TileType surfaceType : {TileType::Grass, TileType::Forest, TileType::Sand})
    {
        for (const RoadOverlayAutotileRole role : kRequiredRoles)
        {
            (void)metadata.getTransitionCell(surfaceType, role);
        }
    }
}

} // namespace

RoadOverlayTilesetMetadata RoadOverlayTilesetMetadata::loadFromFile(const std::filesystem::path& path)
{
    return loadFromDocument(TilesetAssetDocument::loadFromFile(path));
}

RoadOverlayTilesetMetadata RoadOverlayTilesetMetadata::loadFromAssetRoot(
    const std::filesystem::path& assetRoot,
    const std::filesystem::path& catalogRelativePath)
{
    return loadFromDocument(TilesetAssetDocument::loadFromAssetRoot(assetRoot, catalogRelativePath));
}

RoadOverlayTilesetMetadata RoadOverlayTilesetMetadata::loadFromDocument(const TilesetAssetDocument& document)
{
    RoadOverlayTilesetMetadata metadata;

    for (const TilesetAssetTile& tile : document.getTiles())
    {
        if (tile.kind != TilesetAssetTileKind::Overlay || !tile.overlay.has_value())
        {
            continue;
        }

        const RoadOverlayAtlasCell cell{tile.atlas.column, tile.atlas.row};

        if (tile.overlay->overlayClass == "base")
        {
            metadata.m_baseVariants.push_back(cell);
            continue;
        }

        if (tile.overlay->overlayClass == "decor")
        {
            metadata.m_decorVariants.push_back(cell);
            continue;
        }

        if (tile.overlay->overlayClass != "transition")
        {
            continue;
        }

        if (!tile.overlay->onSurface.has_value() || !tile.overlay->autotile.has_value())
        {
            throw std::runtime_error("Road overlay transition entry is missing required metadata");
        }

        metadata.m_transitionCells[parseSurfaceType(*tile.overlay->onSurface)][parseRoadOverlayRole(
            tile.overlay->autotile->role)] = cell;
    }

    validateMetadata(metadata);
    return metadata;
}

std::size_t RoadOverlayTilesetMetadata::getBaseVariantCount() const noexcept
{
    return m_baseVariants.size();
}

const RoadOverlayAtlasCell& RoadOverlayTilesetMetadata::getBaseVariant(const std::size_t variantIndex) const
{
    if (m_baseVariants.empty())
    {
        throw std::runtime_error("Missing road overlay base variants in overlay tileset metadata");
    }

    return m_baseVariants[variantIndex % m_baseVariants.size()];
}

std::size_t RoadOverlayTilesetMetadata::getDecorVariantCount() const noexcept
{
    return m_decorVariants.size();
}

const RoadOverlayAtlasCell& RoadOverlayTilesetMetadata::getDecorVariant(const std::size_t variantIndex) const
{
    if (m_decorVariants.empty())
    {
        throw std::runtime_error("Missing road overlay decor variants in overlay tileset metadata");
    }

    return m_decorVariants[variantIndex % m_decorVariants.size()];
}

const RoadOverlayAtlasCell& RoadOverlayTilesetMetadata::getTransitionCell(
    const TileType surfaceType,
    const RoadOverlayAutotileRole role) const
{
    const auto surfaceIt = m_transitionCells.find(surfaceType);

    if (surfaceIt == m_transitionCells.end())
    {
        throw std::runtime_error("Missing road overlay surface transitions in overlay tileset metadata");
    }

    const auto roleIt = surfaceIt->second.find(role);

    if (roleIt == surfaceIt->second.end())
    {
        throw std::runtime_error("Missing road overlay autotile role in overlay tileset metadata");
    }

    return roleIt->second;
}

} // namespace detail
} // namespace rpg
