/**
 * @file RoadOverlayTilesetSupport.hpp
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

#ifndef RPG_MAIN_ROAD_OVERLAY_TILESET_SUPPORT_HPP
#define RPG_MAIN_ROAD_OVERLAY_TILESET_SUPPORT_HPP

#include <main/World.hpp>

#include "TilesetAssetLoader.hpp"

#include <filesystem>
#include <map>
#include <optional>
#include <vector>

namespace rpg
{
namespace detail
{

enum class RoadOverlayAutotileRole
{
    OuterTopLeft,
    OuterTopRight,
    OuterBottomLeft,
    OuterBottomRight,
    TopLeft,
    Top,
    TopRight,
    Left,
    Right,
    BottomLeft,
    Bottom,
    BottomRight
};

struct RoadOverlayAtlasCell
{
    int tileX = 0;
    int tileY = 0;
};

class RoadOverlayTilesetMetadata
{
public:
    [[nodiscard]] static RoadOverlayTilesetMetadata loadFromFile(const std::filesystem::path& path);
    [[nodiscard]] static RoadOverlayTilesetMetadata loadFromAssetRoot(
        const std::filesystem::path& assetRoot,
        const std::filesystem::path& catalogRelativePath);
    [[nodiscard]] static RoadOverlayTilesetMetadata loadFromDocument(const TilesetAssetDocument& document);

    [[nodiscard]] std::size_t getBaseVariantCount() const noexcept;
    [[nodiscard]] const RoadOverlayAtlasCell& getBaseVariant(std::size_t variantIndex) const;
    [[nodiscard]] std::size_t getDecorVariantCount() const noexcept;
    [[nodiscard]] const RoadOverlayAtlasCell& getDecorVariant(std::size_t variantIndex) const;
    [[nodiscard]] const RoadOverlayAtlasCell& getTransitionCell(TileType surfaceType, RoadOverlayAutotileRole role) const;

private:
    std::vector<RoadOverlayAtlasCell> m_baseVariants;
    std::vector<RoadOverlayAtlasCell> m_decorVariants;
    std::map<TileType, std::map<RoadOverlayAutotileRole, RoadOverlayAtlasCell>> m_transitionCells;
};

} // namespace detail
} // namespace rpg

#endif // RPG_MAIN_ROAD_OVERLAY_TILESET_SUPPORT_HPP
