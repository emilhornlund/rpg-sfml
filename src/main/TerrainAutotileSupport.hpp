/**
 * @file TerrainAutotileSupport.hpp
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

#ifndef RPG_MAIN_TERRAIN_AUTOTILE_SUPPORT_HPP
#define RPG_MAIN_TERRAIN_AUTOTILE_SUPPORT_HPP

#include <main/OverworldRuntime.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <vector>

namespace rpg
{
namespace detail
{

enum class TerrainAutotileRole
{
    OuterTopLeft,
    OuterTopRight,
    OuterBottomLeft,
    OuterBottomRight,
    SingleTile,
    TopLeft,
    Top,
    TopRight,
    Left,
    Center,
    Right,
    BottomLeft,
    Bottom,
    BottomRight
};

struct TerrainAtlasCell
{
    int tileX = 0;
    int tileY = 0;
};

class TerrainTilesetMetadata
{
public:
    [[nodiscard]] static TerrainTilesetMetadata loadFromFile(const std::filesystem::path& path);

    [[nodiscard]] std::size_t getBaseVariantCount(TileType tileType) const noexcept;
    [[nodiscard]] const TerrainAtlasCell& getBaseVariant(TileType tileType, std::size_t variantIndex) const;
    [[nodiscard]] std::size_t getDecorVariantCount(TileType tileType) const noexcept;
    [[nodiscard]] const TerrainAtlasCell& getDecorVariant(TileType tileType, std::size_t variantIndex) const;
    [[nodiscard]] const TerrainAtlasCell& getTransitionCell(
        TileType sourceTileType,
        TileType transitionTarget,
        TerrainAutotileRole role,
        int animationFrame = 0) const;
    [[nodiscard]] const TerrainAtlasCell& getOpenWaterCell(int animationFrame = 0) const;
    [[nodiscard]] int getWaterAnimationFrameCount() const noexcept;

private:
    struct TransitionKey
    {
        TileType sourceTileType = TileType::Water;
        TileType transitionTarget = TileType::Water;

        [[nodiscard]] bool operator<(const TransitionKey& other) const noexcept;
    };

    std::map<TileType, std::vector<TerrainAtlasCell>> m_baseVariants;
    std::map<TileType, std::vector<TerrainAtlasCell>> m_decorVariants;
    std::map<TransitionKey, std::map<TerrainAutotileRole, std::map<int, TerrainAtlasCell>>> m_transitionCells;
};

struct TerrainAppearanceSelection
{
    bool useDecor = false;
    std::size_t variantIndex = 0U;
};

[[nodiscard]] constexpr bool supportsAutotileTransition(
    const TileType sourceTileType,
    const TileType transitionTarget) noexcept
{
    switch (sourceTileType)
    {
    case TileType::Grass:
        return transitionTarget == TileType::Forest
            || transitionTarget == TileType::Sand
            || transitionTarget == TileType::Water;
    case TileType::Forest:
        return transitionTarget == TileType::Sand
            || transitionTarget == TileType::Water;
    case TileType::Sand:
        return transitionTarget == TileType::Water;
    case TileType::Water:
        return false;
    }

    return false;
}

[[nodiscard]] constexpr int getTerrainPriority(const TileType tileType) noexcept
{
    switch (tileType)
    {
    case TileType::Water:
        return 4;
    case TileType::Sand:
        return 3;
    case TileType::Forest:
        return 2;
    case TileType::Grass:
        return 1;
    }

    return 0;
}

[[nodiscard]] std::optional<TileType> selectAutotileTransitionTarget(
    TileType currentTileType,
    const std::array<TileType, 8>& neighborTileTypes) noexcept;

[[nodiscard]] std::optional<TerrainAutotileRole> resolveAutotileRole(
    TileType currentTileType,
    TileType transitionSource,
    const std::array<TileType, 8>& neighborTileTypes) noexcept;

[[nodiscard]] std::size_t selectTerrainVariantIndex(
    std::uint32_t seed,
    const TileCoordinates& coordinates,
    TileType tileType,
    std::size_t variantCount,
    std::uint32_t salt = 0U) noexcept;

[[nodiscard]] TerrainAppearanceSelection selectTerrainAppearanceSelection(
    std::uint32_t seed,
    const TileCoordinates& coordinates,
    TileType tileType,
    std::size_t baseVariantCount,
    std::size_t decorVariantCount) noexcept;

[[nodiscard]] int selectWaterAnimationFrame(float elapsedSeconds, int frameCount) noexcept;

[[nodiscard]] TerrainAtlasCell selectTerrainAtlasCell(
    const TerrainTilesetMetadata& metadata,
    const OverworldRenderTile& tile,
    const std::array<TileType, 8>& neighborTileTypes,
    float animationElapsedSeconds,
    std::uint32_t seed);

} // namespace detail
} // namespace rpg

#endif // RPG_MAIN_TERRAIN_AUTOTILE_SUPPORT_HPP
