/**
 * @file VegetationAtlasSupport.hpp
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

#ifndef RPG_MAIN_VEGETATION_ATLAS_SUPPORT_HPP
#define RPG_MAIN_VEGETATION_ATLAS_SUPPORT_HPP

#include "TilesetAssetLoader.hpp"

#include <filesystem>
#include <utility>
#include <string>
#include <vector>

namespace rpg
{
namespace detail
{

struct VegetationAtlasCell
{
    int tileX = 0;
    int tileY = 0;
};

struct VegetationAtlasPart
{
    VegetationAtlasCell cell{};
    int offsetX = 0;
    int offsetY = 0;
    bool isAnchor = false;
};

struct VegetationPrototypeBounds
{
    int minOffsetX = 0;
    int maxOffsetX = 0;
    int minOffsetY = 0;
    int maxOffsetY = 0;
};

struct VegetationPrototype
{
    std::string id;
    std::string family;
    std::vector<std::string> tags;
    std::vector<VegetationAtlasPart> parts;
    VegetationPrototypeBounds bounds{};
    VegetationPlacementMode placementMode = VegetationPlacementMode::GroundDense;
    std::vector<std::string> placeOn;
    std::vector<std::pair<std::string, float>> biomes;
};

class VegetationTilesetMetadata
{
public:
    [[nodiscard]] static VegetationTilesetMetadata loadFromFile(const std::filesystem::path& path);
    [[nodiscard]] static VegetationTilesetMetadata loadFromAssetRoot(
        const std::filesystem::path& assetRoot,
        const std::filesystem::path& classificationRelativePath);
    [[nodiscard]] static VegetationTilesetMetadata loadFromDocument(const TilesetAssetDocument& document);

    [[nodiscard]] std::size_t getPrototypeCount() const noexcept;
    [[nodiscard]] const VegetationPrototype& getPrototype(std::size_t index) const;
    [[nodiscard]] const VegetationPrototype& getPrototypeById(const std::string& id) const;
    [[nodiscard]] int getMaxPrototypeWidthInTiles() const noexcept;
    [[nodiscard]] int getMaxPrototypeHeightInTiles() const noexcept;

private:
    std::vector<VegetationPrototype> m_prototypes;
    int m_maxPrototypeWidthInTiles = 0;
    int m_maxPrototypeHeightInTiles = 0;
};

} // namespace detail
} // namespace rpg

#endif // RPG_MAIN_VEGETATION_ATLAS_SUPPORT_HPP
