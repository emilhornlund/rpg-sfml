/**
 * @file GameAssetSupportTests.cpp
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

#include "GameAssetSupport.hpp"

#include <algorithm>
#include <filesystem>

namespace
{

[[nodiscard]] bool verifyAssetPathResolution()
{
    const std::filesystem::path fakeExecutableDirectory = "/tmp/rpg-sfml-build/src/main";
    const std::filesystem::path assetRoot = rpg::detail::getAssetRootPath(fakeExecutableDirectory);

    return assetRoot == fakeExecutableDirectory / "assets"
        && rpg::detail::getTerrainTilesetPath(assetRoot) == assetRoot / "overworld-terrain-tileset.png"
        && rpg::detail::getVegetationTilesetPath(assetRoot) == assetRoot / "overworld-vegetation-tileset.png"
        && rpg::detail::getPlayerSpritesheetPath(assetRoot) == assetRoot / "player-walking-spritesheet.png"
        && rpg::detail::getTerrainTilesetClassificationPath(assetRoot)
            == assetRoot / "output/classifications/overworld-terrain-tileset-classification.json"
        && rpg::detail::getVegetationTilesetClassificationPath(assetRoot)
            == assetRoot / "output/classifications/overworld-vegetation-tileset-classification.json"
        && rpg::detail::getDebugOverlayFontPath(assetRoot) == assetRoot / "output/fonts/PixelOperator8.ttf";
}

[[nodiscard]] bool verifyTerrainMetadataLoading(const std::filesystem::path& assetRoot)
{
    const rpg::detail::TerrainTilesetMetadata metadata = rpg::detail::loadTerrainTilesetMetadata(assetRoot);
    const rpg::detail::TerrainAtlasCell topTransition = metadata.getTransitionCell(
        rpg::TileType::Grass,
        rpg::TileType::Sand,
        rpg::detail::TerrainAutotileRole::Top);

    return metadata.getBaseVariantCount(rpg::TileType::Grass) == 5
        && metadata.getDecorVariantCount(rpg::TileType::Forest) == 13
        && metadata.getWaterAnimationFrameCount() == 3
        && topTransition.tileX == 1
        && topTransition.tileY == 5;
}

[[nodiscard]] bool verifyVegetationMetadataLoading(const std::filesystem::path& assetRoot)
{
    const rpg::detail::VegetationTilesetMetadata metadata = rpg::detail::loadVegetationTilesetMetadata(assetRoot);
    const rpg::detail::VegetationPrototype& darkOak = metadata.getPrototypeById("oak_tree_large_dark_1");
    const rpg::detail::VegetationPrototype& bush = metadata.getPrototypeById("bush_small_1");
    const rpg::detail::VegetationPrototype& waterLily = metadata.getPrototypeById("water_lily_1");
    const auto containsPlacementClass = [](const rpg::detail::VegetationPrototype& prototype, const char* value)
    {
        return std::find(prototype.placeOn.begin(), prototype.placeOn.end(), value) != prototype.placeOn.end();
    };
    const auto containsBiomeWeight = [](const rpg::detail::VegetationPrototype& prototype, const char* key, const float weight)
    {
        const auto biomeIt = std::find_if(
            prototype.biomes.begin(),
            prototype.biomes.end(),
            [key](const auto& entry)
            {
                return entry.first == key;
            });
        return biomeIt != prototype.biomes.end() && biomeIt->second == weight;
    };

    return metadata.getPrototypeCount() == 25
        && metadata.getMaxPrototypeWidthInTiles() == 6
        && metadata.getMaxPrototypeHeightInTiles() == 8
        && darkOak.family == "tree"
        && darkOak.bounds.minOffsetX == -3
        && darkOak.bounds.maxOffsetX == 2
        && darkOak.bounds.minOffsetY == -7
        && darkOak.bounds.maxOffsetY == 0
        && darkOak.parts.size() == 39
        && darkOak.placeOn.size() == 1
        && containsPlacementClass(darkOak, "forest")
        && darkOak.biomes.size() == 1
        && containsBiomeWeight(darkOak, "forest", 1.0F)
        && bush.bounds.minOffsetX == 0
        && bush.bounds.maxOffsetX == 1
        && bush.bounds.minOffsetY == -1
        && bush.bounds.maxOffsetY == 0
        && bush.parts.size() == 4
        && bush.placeOn.size() == 2
        && containsPlacementClass(bush, "grass")
        && containsPlacementClass(bush, "forest")
        && bush.biomes.size() == 2
        && containsBiomeWeight(bush, "forest", 0.01F)
        && containsBiomeWeight(bush, "grass", 0.1F)
        && waterLily.placeOn.size() == 1
        && containsPlacementClass(waterLily, "water")
        && waterLily.biomes.size() == 1
        && containsBiomeWeight(waterLily, "water", 0.25F);
}

[[nodiscard]] bool verifyDebugOverlayFontPresence(const std::filesystem::path& assetRoot)
{
    const std::filesystem::path fontPath = rpg::detail::getDebugOverlayFontPath(assetRoot);

    return std::filesystem::exists(fontPath)
        && std::filesystem::is_regular_file(fontPath)
        && std::filesystem::file_size(fontPath) > 0;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 1;
    }

    if (!verifyAssetPathResolution())
    {
        return 1;
    }

    if (!verifyTerrainMetadataLoading(argv[1]))
    {
        return 1;
    }

    if (!verifyVegetationMetadataLoading(argv[1]))
    {
        return 1;
    }

    if (!verifyDebugOverlayFontPresence(argv[1]))
    {
        return 1;
    }

    return 0;
}
