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

#include <filesystem>

namespace
{

[[nodiscard]] bool verifyAssetPathResolution()
{
    const std::filesystem::path fakeExecutableDirectory = "/tmp/rpg-sfml-build/src/main";
    const std::filesystem::path assetRoot = rpg::detail::getAssetRootPath(fakeExecutableDirectory);

    return assetRoot == fakeExecutableDirectory / "assets"
        && rpg::detail::getTerrainTilesetPath(assetRoot) == assetRoot / "overworld-terrain-tileset.png"
        && rpg::detail::getPlayerSpritesheetPath(assetRoot) == assetRoot / "player-walking-spritesheet.png"
        && rpg::detail::getTerrainTilesetClassificationPath(assetRoot)
            == assetRoot / "output/classifications/overworld-terrain-tileset-classification.json";
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

    return 0;
}
