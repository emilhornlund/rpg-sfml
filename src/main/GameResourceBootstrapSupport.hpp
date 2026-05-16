/**
 * @file GameResourceBootstrapSupport.hpp
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

#ifndef RPG_MAIN_GAME_RESOURCE_BOOTSTRAP_SUPPORT_HPP
#define RPG_MAIN_GAME_RESOURCE_BOOTSTRAP_SUPPORT_HPP

#include "GameAssetSupport.hpp"

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <filesystem>

namespace rpg::detail
{

struct GameRenderResources
{
    sf::Texture terrainTileset;
    TerrainTilesetMetadata terrainTilesetMetadata;
    sf::Texture groundOverlayTileset;
    RoadOverlayTilesetMetadata groundOverlayTilesetMetadata;
    sf::Texture vegetationTileset;
    VegetationTilesetMetadata vegetationTilesetMetadata;
    sf::Texture playerSpritesheet;
    sf::Shader playerOcclusionShader;
};

[[nodiscard]] GameRenderResources loadGameRenderResources(const std::filesystem::path& assetRoot);

[[nodiscard]] GameRenderResources loadGameRenderResources();

} // namespace rpg::detail

#endif // RPG_MAIN_GAME_RESOURCE_BOOTSTRAP_SUPPORT_HPP
