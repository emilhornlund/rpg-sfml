/**
 * @file GameAssetSupport.hpp
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

#ifndef RPG_MAIN_GAME_ASSET_SUPPORT_HPP
#define RPG_MAIN_GAME_ASSET_SUPPORT_HPP

#include "TerrainAutotileSupport.hpp"

#include <filesystem>

namespace rpg::detail
{

inline constexpr char kAssetDirectoryName[] = "assets";
inline constexpr char kTerrainTilesetFilename[] = "overworld-terrain-tileset.png";
inline constexpr char kTerrainTilesetClassificationFilename[] = "overworld-terrain-tileset-classification.json";
inline constexpr char kTerrainTilesetClassificationDirectory[] = "output/classifications";
inline constexpr char kPlayerSpritesheetFilename[] = "player-walking-spritesheet.png";

[[nodiscard]] inline std::filesystem::path getExecutableDirectory()
{
    return std::filesystem::read_symlink("/proc/self/exe").parent_path();
}

[[nodiscard]] inline std::filesystem::path getAssetRootPath(const std::filesystem::path& executableDirectory)
{
    return executableDirectory / kAssetDirectoryName;
}

[[nodiscard]] inline std::filesystem::path getAssetRootPath()
{
    return getAssetRootPath(getExecutableDirectory());
}

[[nodiscard]] inline std::filesystem::path getTerrainTilesetPath(const std::filesystem::path& assetRoot)
{
    return assetRoot / kTerrainTilesetFilename;
}

[[nodiscard]] inline std::filesystem::path getPlayerSpritesheetPath(const std::filesystem::path& assetRoot)
{
    return assetRoot / kPlayerSpritesheetFilename;
}

[[nodiscard]] inline std::filesystem::path getTerrainTilesetClassificationPath(const std::filesystem::path& assetRoot)
{
    return assetRoot / kTerrainTilesetClassificationDirectory / kTerrainTilesetClassificationFilename;
}

[[nodiscard]] inline TerrainTilesetMetadata loadTerrainTilesetMetadata(const std::filesystem::path& assetRoot)
{
    return TerrainTilesetMetadata::loadFromAssetRoot(
        assetRoot,
        std::filesystem::path(kTerrainTilesetClassificationDirectory) / kTerrainTilesetClassificationFilename);
}

} // namespace rpg::detail

#endif // RPG_MAIN_GAME_ASSET_SUPPORT_HPP
