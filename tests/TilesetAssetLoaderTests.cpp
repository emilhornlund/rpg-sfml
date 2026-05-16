/**
 * @file TilesetAssetLoaderTests.cpp
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

#include "TilesetAssetLoader.hpp"

#include <filesystem>

namespace
{

[[nodiscard]] bool verifyTerrainDocument(const std::filesystem::path& assetRoot)
{
    const rpg::detail::TilesetAssetDocument terrain = rpg::detail::TilesetAssetDocument::loadFromAssetRoot(
        assetRoot,
        "output/catalogs/overworld-terrain-tileset-catalog.json");
    bool foundGrassBase = false;

    for (const rpg::detail::TilesetAssetTile& tile : terrain.getTiles())
    {
        if (tile.kind != rpg::detail::TilesetAssetTileKind::Terrain || !tile.terrain.has_value())
        {
            continue;
        }

        if (tile.terrain->id == "grass" && tile.terrain->classification == "base" && tile.atlas.column == 0 && tile.atlas.row == 0)
        {
            foundGrassBase = true;
            break;
        }
    }

    return terrain.getTileset().id == "overworld-terrain-tileset"
        && terrain.getTileset().source.image == "assets/source/tilesets/overworld-terrain/overworld-terrain-tileset.png"
        && terrain.getTileset().runtime.image == "output/tilesets/overworld-terrain-tileset.png"
        && terrain.getTileset().grid.tileSize == 16
        && terrain.getTileset().grid.tileCount == 256
        && terrain.getResolvedImagePath() == assetRoot / "output/tilesets/overworld-terrain-tileset.png"
        && foundGrassBase;
}

[[nodiscard]] bool verifyVegetationDocument(const std::filesystem::path& assetRoot)
{
    const rpg::detail::TilesetAssetDocument vegetation = rpg::detail::TilesetAssetDocument::loadFromAssetRoot(
        assetRoot,
        "output/catalogs/overworld-vegetation-tileset-catalog.json");
    bool foundObjectTile = false;
    bool foundWaterLilyAnchor = false;
    bool foundPropAnchor = false;
    bool foundTreeAnchor = false;

    for (const rpg::detail::TilesetAssetTile& tile : vegetation.getTiles())
    {
        if (tile.kind != rpg::detail::TilesetAssetTileKind::Object || !tile.object.has_value())
        {
            continue;
        }

        if (tile.object->id == "fallen_log_large_1"
            && tile.object->role == "part"
            && tile.object->offsetX == 0
            && tile.object->offsetY == -1)
        {
            foundObjectTile = true;
        }

        if (tile.object->id == "water_lily_1"
            && tile.object->role == "anchor"
            && tile.object->placementMode.has_value()
            && *tile.object->placementMode == rpg::detail::VegetationPlacementMode::GroundDense
            && tile.object->placeOn.size() == 1
            && tile.object->placeOn.front() == "water"
            && tile.object->biomes.size() == 1
            && tile.object->biomes.front().first == "water"
            && tile.object->biomes.front().second == 0.25F)
        {
            foundWaterLilyAnchor = true;
        }

        if (tile.object->id == "stump_cut_large_1"
            && tile.object->role == "anchor"
            && tile.object->placementMode.has_value()
            && *tile.object->placementMode == rpg::detail::VegetationPlacementMode::PropSparse)
        {
            foundPropAnchor = true;
        }

        if (tile.object->id == "round_tree_small_1"
            && tile.object->role == "anchor"
            && tile.object->placementMode.has_value()
            && *tile.object->placementMode == rpg::detail::VegetationPlacementMode::TreeSparse)
        {
            foundTreeAnchor = true;
        }
    }

    return vegetation.getTileset().id == "overworld-vegetation-tileset"
        && vegetation.getTileset().source.image
            == "assets/source/tilesets/overworld-vegetation/overworld-vegetation-tileset.png"
        && vegetation.getTileset().runtime.image == "output/tilesets/overworld-vegetation-tileset.png"
        && vegetation.getTileset().grid.tileSize == 16
        && vegetation.getTileset().grid.tileCount == 225
        && vegetation.getResolvedImagePath() == assetRoot / "output/tilesets/overworld-vegetation-tileset.png"
        && foundObjectTile
        && foundWaterLilyAnchor
        && foundPropAnchor
        && foundTreeAnchor;
}

[[nodiscard]] bool verifyGroundOverlayDocument(const std::filesystem::path& assetRoot)
{
    const rpg::detail::TilesetAssetDocument overlay = rpg::detail::TilesetAssetDocument::loadFromAssetRoot(
        assetRoot,
        "output/catalogs/overworld-ground-overlay-tileset-catalog.json");
    bool foundBaseTile = false;
    bool foundTransitionTile = false;

    for (const rpg::detail::TilesetAssetTile& tile : overlay.getTiles())
    {
        if (tile.kind != rpg::detail::TilesetAssetTileKind::Overlay || !tile.overlay.has_value())
        {
            continue;
        }

        if (tile.overlay->id == "dirt_road"
            && tile.overlay->overlayClass == "base"
            && tile.overlay->variant.has_value()
            && *tile.overlay->variant == "base_1"
            && tile.atlas.column == 0
            && tile.atlas.row == 0)
        {
            foundBaseTile = true;
        }

        if (tile.overlay->id == "dirt_road"
            && tile.overlay->overlayClass == "transition"
            && tile.overlay->onSurface.has_value()
            && *tile.overlay->onSurface == "grass"
            && tile.overlay->autotile.has_value()
            && tile.overlay->autotile->role == "top"
            && tile.atlas.column == 6
            && tile.atlas.row == 2)
        {
            foundTransitionTile = true;
        }
    }

    return overlay.getTileset().id == "overworld-ground-overlay-tileset"
        && overlay.getTileset().runtime.image == "output/tilesets/overworld-ground-overlay-tileset.png"
        && overlay.getTileset().grid.tileSize == 16
        && overlay.getTileset().grid.tileCount == 60
        && overlay.getResolvedImagePath() == assetRoot / "output/tilesets/overworld-ground-overlay-tileset.png"
        && foundBaseTile
        && foundTransitionTile;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 1;
    }

    const std::filesystem::path assetRoot = argv[1];

    if (!verifyTerrainDocument(assetRoot))
    {
        return 1;
    }

    if (!verifyVegetationDocument(assetRoot))
    {
        return 1;
    }

    if (!verifyGroundOverlayDocument(assetRoot))
    {
        return 1;
    }

    return 0;
}
