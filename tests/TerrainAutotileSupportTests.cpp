/**
 * @file TerrainAutotileSupportTests.cpp
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

#include "TerrainAutotileSupport.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>

namespace
{

[[nodiscard]] bool verifyMetadataLoading(const std::filesystem::path& classificationPath)
{
    const rpg::detail::TerrainTilesetMetadata metadata =
        rpg::detail::TerrainTilesetMetadata::loadFromFile(classificationPath);

    const rpg::detail::TerrainAtlasCell grassTop = metadata.getTransitionCell(
        rpg::TileType::Grass,
        rpg::TileType::Sand,
        rpg::detail::TerrainAutotileRole::Top);
    const rpg::detail::TerrainAtlasCell forestCorner = metadata.getTransitionCell(
        rpg::TileType::Grass,
        rpg::TileType::Forest,
        rpg::detail::TerrainAutotileRole::OuterTopLeft);
    const rpg::detail::TerrainAtlasCell waterFrameOne = metadata.getTransitionCell(
        rpg::TileType::Grass,
        rpg::TileType::Water,
        rpg::detail::TerrainAutotileRole::Center,
        1);
    const rpg::detail::TerrainAtlasCell grassDecor = metadata.getDecorVariant(
        rpg::TileType::Grass,
        0);

    return metadata.getBaseVariantCount(rpg::TileType::Grass) == 5
        && metadata.getBaseVariantCount(rpg::TileType::Sand) == 6
        && metadata.getBaseVariantCount(rpg::TileType::Forest) == 6
        && metadata.getDecorVariantCount(rpg::TileType::Grass) == 38
        && metadata.getDecorVariantCount(rpg::TileType::Forest) == 13
        && metadata.getDecorVariantCount(rpg::TileType::Sand) == 20
        && metadata.getWaterAnimationFrameCount() == 3
        && grassTop.tileX == 1
        && grassTop.tileY == 5
        && forestCorner.tileX == 3
        && forestCorner.tileY == 3
        && grassDecor.tileX == 0
        && grassDecor.tileY == 1
        && waterFrameOne.tileX == 10
        && waterFrameOne.tileY == 6;
}

[[nodiscard]] bool verifyTransitionPriorityAndRoleSelection()
{
    const std::array<rpg::TileType, 8> mixedWaterNeighbors{
        rpg::TileType::Forest,
        rpg::TileType::Forest,
        rpg::TileType::Sand,
        rpg::TileType::Water,
        rpg::TileType::Water,
        rpg::TileType::Water,
        rpg::TileType::Forest,
        rpg::TileType::Forest};
    const std::array<rpg::TileType, 8> diagonalCornerNeighbors{
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Grass};
    const std::array<rpg::TileType, 8> innerCornerNeighbors{
        rpg::TileType::Grass,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Grass,
        rpg::TileType::Sand};
    const std::array<rpg::TileType, 8> topBandNeighbors{
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Grass};
    const std::array<rpg::TileType, 8> isolatedTargetNeighbors{
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass,
        rpg::TileType::Grass};
    const std::array<rpg::TileType, 8> mixedSourceSurroundNeighbors{
        rpg::TileType::Grass,
        rpg::TileType::Forest,
        rpg::TileType::Grass,
        rpg::TileType::Forest,
        rpg::TileType::Grass,
        rpg::TileType::Forest,
        rpg::TileType::Grass,
        rpg::TileType::Forest};
    const std::array<rpg::TileType, 8> noSourceNeighbors{
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand};
    const std::array<rpg::TileType, 8> mixedBiomeCornerNeighbors{
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Forest,
        rpg::TileType::Sand,
        rpg::TileType::Grass,
        rpg::TileType::Sand};
    const std::array<rpg::TileType, 8> diagonalBottomPairNeighbors{
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Sand,
        rpg::TileType::Grass,
        rpg::TileType::Sand,
        rpg::TileType::Grass};
    const std::array<rpg::TileType, 8> diagonalLeftPairNeighbors{
        rpg::TileType::Water,
        rpg::TileType::Water,
        rpg::TileType::Water,
        rpg::TileType::Water,
        rpg::TileType::Water,
        rpg::TileType::Grass,
        rpg::TileType::Water,
        rpg::TileType::Grass};

    const auto selectedWaterTarget =
        rpg::detail::selectAutotileTransitionTarget(rpg::TileType::Water, mixedWaterNeighbors);
    const auto diagonalCornerTarget =
        rpg::detail::selectAutotileTransitionTarget(rpg::TileType::Sand, diagonalCornerNeighbors);
    const auto noSourceTransitionTarget =
        rpg::detail::selectAutotileTransitionTarget(rpg::TileType::Grass, noSourceNeighbors);
    const auto mixedBiomeCornerTarget =
        rpg::detail::selectAutotileTransitionTarget(rpg::TileType::Sand, mixedBiomeCornerNeighbors);
    const auto mixedSourceSurroundTarget =
        rpg::detail::selectAutotileTransitionTarget(rpg::TileType::Sand, mixedSourceSurroundNeighbors);

    return selectedWaterTarget.has_value()
        && *selectedWaterTarget == rpg::TileType::Forest
        && rpg::detail::resolveAutotileRole(rpg::TileType::Water, *selectedWaterTarget, mixedWaterNeighbors).has_value()
        && *rpg::detail::resolveAutotileRole(rpg::TileType::Water, *selectedWaterTarget, mixedWaterNeighbors)
            == rpg::detail::TerrainAutotileRole::Top
        && diagonalCornerTarget.has_value()
        && *diagonalCornerTarget == rpg::TileType::Grass
        && rpg::detail::resolveAutotileRole(rpg::TileType::Sand, *diagonalCornerTarget, diagonalCornerNeighbors).has_value()
        && *rpg::detail::resolveAutotileRole(rpg::TileType::Sand, *diagonalCornerTarget, diagonalCornerNeighbors)
            == rpg::detail::TerrainAutotileRole::OuterTopLeft
        && rpg::detail::resolveAutotileRole(rpg::TileType::Sand, rpg::TileType::Grass, innerCornerNeighbors).has_value()
        && *rpg::detail::resolveAutotileRole(rpg::TileType::Sand, rpg::TileType::Grass, innerCornerNeighbors)
            == rpg::detail::TerrainAutotileRole::TopLeft
        && rpg::detail::resolveAutotileRole(rpg::TileType::Sand, rpg::TileType::Grass, topBandNeighbors).has_value()
        && *rpg::detail::resolveAutotileRole(rpg::TileType::Sand, rpg::TileType::Grass, topBandNeighbors)
            == rpg::detail::TerrainAutotileRole::Top
        && rpg::detail::resolveAutotileRole(rpg::TileType::Sand, rpg::TileType::Grass, isolatedTargetNeighbors).has_value()
        && *rpg::detail::resolveAutotileRole(rpg::TileType::Sand, rpg::TileType::Grass, isolatedTargetNeighbors)
            == rpg::detail::TerrainAutotileRole::SingleTile
        && mixedBiomeCornerTarget.has_value()
        && *mixedBiomeCornerTarget == rpg::TileType::Forest
        && rpg::detail::resolveAutotileRole(rpg::TileType::Sand, *mixedBiomeCornerTarget, mixedBiomeCornerNeighbors).has_value()
        && *rpg::detail::resolveAutotileRole(rpg::TileType::Sand, *mixedBiomeCornerTarget, mixedBiomeCornerNeighbors)
            == rpg::detail::TerrainAutotileRole::BottomLeft
        && mixedSourceSurroundTarget.has_value()
        && *mixedSourceSurroundTarget == rpg::TileType::Grass
        && rpg::detail::resolveAutotileRole(rpg::TileType::Sand, *mixedSourceSurroundTarget, mixedSourceSurroundNeighbors).has_value()
        && *rpg::detail::resolveAutotileRole(rpg::TileType::Sand, *mixedSourceSurroundTarget, mixedSourceSurroundNeighbors)
            == rpg::detail::TerrainAutotileRole::Center
        && !rpg::detail::resolveAutotileRole(
                rpg::TileType::Sand,
                rpg::TileType::Grass,
                diagonalBottomPairNeighbors)
                .has_value()
        && !rpg::detail::resolveAutotileRole(
                rpg::TileType::Water,
                rpg::TileType::Grass,
                diagonalLeftPairNeighbors)
                .has_value()
        && !noSourceTransitionTarget.has_value();
}

[[nodiscard]] bool verifyVariantAndAnimationSelection(const std::filesystem::path& classificationPath)
{
    constexpr std::uint32_t kWorldSeed = 0x12345678U;
    const rpg::detail::TerrainTilesetMetadata metadata =
        rpg::detail::TerrainTilesetMetadata::loadFromFile(classificationPath);
    const rpg::TileCoordinates coordinates{12, -7};
    const std::size_t firstVariantIndex = rpg::detail::selectTerrainVariantIndex(
        kWorldSeed,
        coordinates,
        rpg::TileType::Forest,
        metadata.getBaseVariantCount(rpg::TileType::Forest));
    const std::size_t repeatedVariantIndex = rpg::detail::selectTerrainVariantIndex(
        kWorldSeed,
        coordinates,
        rpg::TileType::Forest,
        metadata.getBaseVariantCount(rpg::TileType::Forest));
    const rpg::detail::TerrainAppearanceSelection firstAppearanceSelection =
        rpg::detail::selectTerrainAppearanceSelection(
            kWorldSeed,
            coordinates,
            rpg::TileType::Forest,
            metadata.getBaseVariantCount(rpg::TileType::Forest),
            metadata.getDecorVariantCount(rpg::TileType::Forest));
    const rpg::detail::TerrainAppearanceSelection repeatedAppearanceSelection =
        rpg::detail::selectTerrainAppearanceSelection(
            kWorldSeed,
            coordinates,
            rpg::TileType::Forest,
            metadata.getBaseVariantCount(rpg::TileType::Forest),
            metadata.getDecorVariantCount(rpg::TileType::Forest));
    const int secondAnimationFrame = rpg::detail::selectWaterAnimationFrame(0.25F, metadata.getWaterAnimationFrameCount());
    const rpg::OverworldRenderTile isolatedSandTile{
        {8, 5},
        rpg::TileType::Sand,
        {16.0F, 16.0F},
        {8.0F, 8.0F},
        {136.0F, 88.0F}};
    const rpg::OverworldRenderTile openWaterTile{
        {4, 9},
        rpg::TileType::Water,
        {16.0F, 16.0F},
        {8.0F, 8.0F},
        {72.0F, 152.0F}};
    const rpg::detail::TerrainAtlasCell openWaterCell = rpg::detail::selectTerrainAtlasCell(
        metadata,
        openWaterTile,
        {
            rpg::TileType::Water,
            rpg::TileType::Water,
            rpg::TileType::Water,
            rpg::TileType::Water,
            rpg::TileType::Water,
            rpg::TileType::Water,
            rpg::TileType::Water,
            rpg::TileType::Water},
        0.45F,
        kWorldSeed);
    const rpg::detail::TerrainAtlasCell isolatedSandCell = rpg::detail::selectTerrainAtlasCell(
        metadata,
        isolatedSandTile,
        {
            rpg::TileType::Grass,
            rpg::TileType::Grass,
            rpg::TileType::Grass,
            rpg::TileType::Grass,
            rpg::TileType::Grass,
            rpg::TileType::Grass,
            rpg::TileType::Grass,
            rpg::TileType::Grass},
        0.45F,
        kWorldSeed);
    const rpg::detail::TerrainAtlasCell isolatedGrassCell = rpg::detail::selectTerrainAtlasCell(
        metadata,
        {
            {11, 6},
            rpg::TileType::Grass,
            {16.0F, 16.0F},
            {8.0F, 8.0F},
            {184.0F, 104.0F}},
        {
            rpg::TileType::Sand,
            rpg::TileType::Sand,
            rpg::TileType::Sand,
            rpg::TileType::Sand,
            rpg::TileType::Sand,
            rpg::TileType::Sand,
            rpg::TileType::Sand,
            rpg::TileType::Sand},
        0.45F,
        kWorldSeed);
    const rpg::detail::TerrainAppearanceSelection expectedGrassSelection = rpg::detail::selectTerrainAppearanceSelection(
        kWorldSeed,
        {11, 6},
        rpg::TileType::Grass,
        metadata.getBaseVariantCount(rpg::TileType::Grass),
        metadata.getDecorVariantCount(rpg::TileType::Grass));
    const rpg::detail::TerrainAtlasCell expectedGrassCell = expectedGrassSelection.useDecor
        ? metadata.getDecorVariant(rpg::TileType::Grass, expectedGrassSelection.variantIndex)
        : metadata.getBaseVariant(rpg::TileType::Grass, expectedGrassSelection.variantIndex);
    bool foundDecorSelection = false;
    rpg::detail::TerrainAtlasCell decoratedGrassCell{};

    for (int y = -16; y <= 16 && !foundDecorSelection; ++y)
    {
        for (int x = -16; x <= 16 && !foundDecorSelection; ++x)
        {
            const rpg::TileCoordinates decorCoordinates{x, y};
            const rpg::detail::TerrainAppearanceSelection decorSelection = rpg::detail::selectTerrainAppearanceSelection(
                kWorldSeed,
                decorCoordinates,
                rpg::TileType::Grass,
                metadata.getBaseVariantCount(rpg::TileType::Grass),
                metadata.getDecorVariantCount(rpg::TileType::Grass));

            if (!decorSelection.useDecor)
            {
                continue;
            }

            decoratedGrassCell = rpg::detail::selectTerrainAtlasCell(
                metadata,
                {
                    decorCoordinates,
                    rpg::TileType::Grass,
                    {16.0F, 16.0F},
                    {8.0F, 8.0F},
                    {0.0F, 0.0F}},
                {
                    rpg::TileType::Grass,
                    rpg::TileType::Grass,
                    rpg::TileType::Grass,
                    rpg::TileType::Grass,
                    rpg::TileType::Grass,
                    rpg::TileType::Grass,
                    rpg::TileType::Grass,
                    rpg::TileType::Grass},
                0.45F,
                kWorldSeed);
            const rpg::detail::TerrainAtlasCell expectedDecorCell =
                metadata.getDecorVariant(rpg::TileType::Grass, decorSelection.variantIndex);
            foundDecorSelection = decoratedGrassCell.tileX == expectedDecorCell.tileX
                && decoratedGrassCell.tileY == expectedDecorCell.tileY;
        }
    }

    return firstVariantIndex == repeatedVariantIndex
        && firstVariantIndex < metadata.getBaseVariantCount(rpg::TileType::Forest)
        && firstAppearanceSelection.useDecor == repeatedAppearanceSelection.useDecor
        && firstAppearanceSelection.variantIndex == repeatedAppearanceSelection.variantIndex
        && secondAnimationFrame == 1
        && isolatedSandCell.tileX == 2
        && isolatedSandCell.tileY == 3
        && isolatedGrassCell.tileX == expectedGrassCell.tileX
        && isolatedGrassCell.tileY == expectedGrassCell.tileY
        && openWaterCell.tileX == 13
        && openWaterCell.tileY == 6
        && foundDecorSelection;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 1;
    }

    const std::filesystem::path classificationPath = argv[1];

    if (!verifyMetadataLoading(classificationPath))
    {
        return 1;
    }

    if (!verifyTransitionPriorityAndRoleSelection())
    {
        return 1;
    }

    if (!verifyVariantAndAnimationSelection(classificationPath))
    {
        return 1;
    }

    return 0;
}
