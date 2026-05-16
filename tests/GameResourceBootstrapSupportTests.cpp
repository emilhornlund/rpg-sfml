/**
 * @file GameResourceBootstrapSupportTests.cpp
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

#include "GameResourceBootstrapSupport.hpp"

#include <filesystem>

namespace
{

[[nodiscard]] bool verifyGameRenderResourcesLoad(const std::filesystem::path& assetRoot)
{
    const rpg::detail::GameRenderResources resources = rpg::detail::loadGameRenderResources(assetRoot);

    return resources.terrainTileset.getSize().x > 0U
        && resources.terrainTileset.getSize().y > 0U
        && !resources.terrainTileset.isSmooth()
        && resources.terrainTilesetMetadata.getBaseVariantCount(rpg::TileType::Grass) == 5
        && resources.groundOverlayTileset.getSize().x > 0U
        && resources.groundOverlayTileset.getSize().y > 0U
        && !resources.groundOverlayTileset.isSmooth()
        && resources.groundOverlayTilesetMetadata.getBaseVariantCount() == 4
        && resources.groundOverlayTilesetMetadata.getDecorVariantCount() == 20
        && resources.vegetationTileset.getSize().x > 0U
        && resources.vegetationTileset.getSize().y > 0U
        && !resources.vegetationTileset.isSmooth()
        && resources.vegetationTilesetMetadata.getPrototypeCount() == 25
        && resources.playerSpritesheet.getSize().x > 0U
        && resources.playerSpritesheet.getSize().y > 0U
        && !resources.playerSpritesheet.isSmooth();
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 1;
    }

    return verifyGameRenderResourcesLoad(argv[1]) ? 0 : 1;
}
