/**
 * @file TilesetAssetLoader.hpp
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

#ifndef RPG_MAIN_TILESET_ASSET_LOADER_HPP
#define RPG_MAIN_TILESET_ASSET_LOADER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace rpg
{
namespace detail
{

enum class VegetationPlacementMode
{
    TreeSparse,
    GroundDense,
    PropSparse
};

enum class TilesetAssetTileKind
{
    Terrain,
    Object,
    Empty
};

struct TilesetAssetAtlasEntry
{
    int tileId = 0;
    int gid = 0;
    int column = 0;
    int row = 0;
    int pixelX = 0;
    int pixelY = 0;
};

struct TilesetAssetGrid
{
    int tileSize = 0;
    int columns = 0;
    int rows = 0;
    int tileCount = 0;
};

struct TilesetAssetSource
{
    std::string tsx;
    std::string image;
};

struct TilesetAssetMetadata
{
    std::string id;
    TilesetAssetSource source;
    TilesetAssetGrid grid;
};

struct TilesetAssetAutotile
{
    int patternX = 0;
    int patternY = 0;
    std::string role;
};

struct TilesetAssetTerrainData
{
    std::string id;
    std::string classification;
    std::optional<std::string> variant;
    std::optional<std::string> transitionTo;
    std::optional<int> animationFrame;
    std::optional<TilesetAssetAutotile> autotile;
};

struct TilesetAssetObjectData
{
    std::string id;
    std::string role;
    std::string family;
    int offsetX = 0;
    int offsetY = 0;
    std::optional<VegetationPlacementMode> placementMode;
    std::vector<std::string> placeOn;
    std::vector<std::pair<std::string, float>> biomes;
};

struct TilesetAssetTile
{
    TilesetAssetTileKind kind = TilesetAssetTileKind::Empty;
    TilesetAssetAtlasEntry atlas;
    std::string name;
    std::vector<std::string> tags;
    bool walkable = false;
    std::string notes;
    std::optional<TilesetAssetTerrainData> terrain;
    std::optional<TilesetAssetObjectData> object;
};

class TilesetAssetDocument
{
public:
    TilesetAssetDocument() = default;
    TilesetAssetDocument(
        std::filesystem::path assetRoot,
        std::filesystem::path catalogPath,
        std::filesystem::path resolvedImagePath,
        TilesetAssetMetadata tileset,
        std::vector<TilesetAssetTile> tiles);

    [[nodiscard]] static TilesetAssetDocument loadFromFile(const std::filesystem::path& catalogPath);
    [[nodiscard]] static TilesetAssetDocument loadFromAssetRoot(
        const std::filesystem::path& assetRoot,
        const std::filesystem::path& catalogRelativePath);

    [[nodiscard]] const std::filesystem::path& getAssetRoot() const noexcept;
    [[nodiscard]] const std::filesystem::path& getCatalogPath() const noexcept;
    [[nodiscard]] const std::filesystem::path& getResolvedImagePath() const noexcept;
    [[nodiscard]] const TilesetAssetMetadata& getTileset() const noexcept;
    [[nodiscard]] const std::vector<TilesetAssetTile>& getTiles() const noexcept;

private:
    std::filesystem::path m_assetRoot;
    std::filesystem::path m_catalogPath;
    std::filesystem::path m_resolvedImagePath;
    TilesetAssetMetadata m_tileset;
    std::vector<TilesetAssetTile> m_tiles;
};

} // namespace detail
} // namespace rpg

#endif // RPG_MAIN_TILESET_ASSET_LOADER_HPP
