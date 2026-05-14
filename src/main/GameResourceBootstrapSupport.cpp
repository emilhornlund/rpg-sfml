/**
 * @file GameResourceBootstrapSupport.cpp
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

#include "PlayerOcclusionSilhouetteSupport.hpp"

#include <stdexcept>

namespace rpg
{
namespace detail
{
namespace
{

[[nodiscard]] sf::Texture loadTerrainTileset(const std::filesystem::path& assetRoot)
{
    sf::Texture terrainTileset;
    const std::filesystem::path terrainTilesetPath = getTerrainTilesetPath(assetRoot);

    if (!terrainTileset.loadFromFile(terrainTilesetPath.string()))
    {
        throw std::runtime_error("Failed to load overworld terrain tileset from " + terrainTilesetPath.string());
    }

    terrainTileset.setSmooth(false);
    return terrainTileset;
}

[[nodiscard]] sf::Texture loadPlayerSpritesheet(const std::filesystem::path& assetRoot)
{
    sf::Texture playerSpritesheet;
    const std::filesystem::path playerSpritesheetPath = getPlayerSpritesheetPath(assetRoot);

    if (!playerSpritesheet.loadFromFile(playerSpritesheetPath.string()))
    {
        throw std::runtime_error("Failed to load player spritesheet from " + playerSpritesheetPath.string());
    }

    playerSpritesheet.setSmooth(false);
    return playerSpritesheet;
}

[[nodiscard]] sf::Texture loadVegetationTileset(const std::filesystem::path& assetRoot)
{
    sf::Texture vegetationTileset;
    const std::filesystem::path vegetationTilesetPath = getVegetationTilesetPath(assetRoot);

    if (!vegetationTileset.loadFromFile(vegetationTilesetPath.string()))
    {
        throw std::runtime_error("Failed to load overworld vegetation tileset from " + vegetationTilesetPath.string());
    }

    vegetationTileset.setSmooth(false);
    return vegetationTileset;
}

} // namespace

GameRenderResources loadGameRenderResources(const std::filesystem::path& assetRoot)
{
    return {
        loadTerrainTileset(assetRoot),
        loadTerrainTilesetMetadata(assetRoot),
        loadVegetationTileset(assetRoot),
        loadVegetationTilesetMetadata(assetRoot),
        loadPlayerSpritesheet(assetRoot),
        loadPlayerOcclusionShader(),
    };
}

GameRenderResources loadGameRenderResources()
{
    return loadGameRenderResources(getAssetRootPath());
}

} // namespace detail
} // namespace rpg
