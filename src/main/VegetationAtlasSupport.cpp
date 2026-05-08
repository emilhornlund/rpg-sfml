/**
 * @file VegetationAtlasSupport.cpp
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

#include "VegetationAtlasSupport.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <stdexcept>

namespace rpg
{
namespace detail
{

namespace
{

struct PrototypeAccumulator
{
    VegetationPrototype prototype;
    int anchorCount = 0;
    bool hasBounds = false;
};

[[nodiscard]] VegetationAtlasCell makeCell(const TilesetAssetTile& tile) noexcept
{
    return {tile.atlas.column, tile.atlas.row};
}

void appendUniqueTags(std::vector<std::string>& destination, const std::vector<std::string>& source)
{
    for (const std::string& tag : source)
    {
        if (std::find(destination.begin(), destination.end(), tag) == destination.end())
        {
            destination.push_back(tag);
        }
    }
}

void updateBounds(PrototypeAccumulator& accumulator, const int offsetX, const int offsetY) noexcept
{
    if (!accumulator.hasBounds)
    {
        accumulator.prototype.bounds = {offsetX, offsetX, offsetY, offsetY};
        accumulator.hasBounds = true;
        return;
    }

    accumulator.prototype.bounds.minOffsetX = std::min(accumulator.prototype.bounds.minOffsetX, offsetX);
    accumulator.prototype.bounds.maxOffsetX = std::max(accumulator.prototype.bounds.maxOffsetX, offsetX);
    accumulator.prototype.bounds.minOffsetY = std::min(accumulator.prototype.bounds.minOffsetY, offsetY);
    accumulator.prototype.bounds.maxOffsetY = std::max(accumulator.prototype.bounds.maxOffsetY, offsetY);
}

void validatePrototype(const VegetationPrototype& prototype, const int anchorCount)
{
    if (anchorCount != 1)
    {
        throw std::runtime_error("Vegetation prototype must contain exactly one anchor tile: " + prototype.id);
    }

    if (prototype.parts.empty())
    {
        throw std::runtime_error("Vegetation prototype is missing atlas parts: " + prototype.id);
    }
}

} // namespace

VegetationTilesetMetadata VegetationTilesetMetadata::loadFromFile(const std::filesystem::path& path)
{
    return loadFromDocument(TilesetAssetDocument::loadFromFile(path));
}

VegetationTilesetMetadata VegetationTilesetMetadata::loadFromAssetRoot(
    const std::filesystem::path& assetRoot,
    const std::filesystem::path& classificationRelativePath)
{
    return loadFromDocument(TilesetAssetDocument::loadFromAssetRoot(assetRoot, classificationRelativePath));
}

VegetationTilesetMetadata VegetationTilesetMetadata::loadFromDocument(const TilesetAssetDocument& document)
{
    std::map<std::string, PrototypeAccumulator> accumulatedPrototypes;

    for (const TilesetAssetTile& tile : document.getTiles())
    {
        if (tile.kind != TilesetAssetTileKind::Object || !tile.object.has_value())
        {
            continue;
        }

        PrototypeAccumulator& accumulator = accumulatedPrototypes[tile.object->id];

        if (accumulator.prototype.id.empty())
        {
            accumulator.prototype.id = tile.object->id;
            accumulator.prototype.family = tile.object->family;
        }
        else if (accumulator.prototype.family != tile.object->family)
        {
            throw std::runtime_error("Vegetation prototype mixes object families: " + tile.object->id);
        }

        appendUniqueTags(accumulator.prototype.tags, tile.tags);
        accumulator.prototype.parts.push_back({
            makeCell(tile),
            tile.object->offsetX,
            tile.object->offsetY,
            tile.object->role == "anchor"});
        updateBounds(accumulator, tile.object->offsetX, tile.object->offsetY);

        if (tile.object->role == "anchor")
        {
            accumulator.prototype.placeOn = tile.object->placeOn;
            accumulator.prototype.biomes = tile.object->biomes;
            ++accumulator.anchorCount;
        }
    }

    VegetationTilesetMetadata metadata;
    metadata.m_prototypes.reserve(accumulatedPrototypes.size());

    for (auto& [id, accumulator] : accumulatedPrototypes)
    {
        (void)id;
        validatePrototype(accumulator.prototype, accumulator.anchorCount);
        std::sort(
            accumulator.prototype.parts.begin(),
            accumulator.prototype.parts.end(),
            [](const VegetationAtlasPart& lhs, const VegetationAtlasPart& rhs)
            {
                if (lhs.offsetY != rhs.offsetY)
                {
                    return lhs.offsetY < rhs.offsetY;
                }

                return lhs.offsetX < rhs.offsetX;
            });
        metadata.m_maxPrototypeWidthInTiles = std::max(
            metadata.m_maxPrototypeWidthInTiles,
            accumulator.prototype.bounds.maxOffsetX - accumulator.prototype.bounds.minOffsetX + 1);
        metadata.m_maxPrototypeHeightInTiles = std::max(
            metadata.m_maxPrototypeHeightInTiles,
            accumulator.prototype.bounds.maxOffsetY - accumulator.prototype.bounds.minOffsetY + 1);
        metadata.m_prototypes.push_back(std::move(accumulator.prototype));
    }

    std::sort(
        metadata.m_prototypes.begin(),
        metadata.m_prototypes.end(),
        [](const VegetationPrototype& lhs, const VegetationPrototype& rhs)
        {
            return lhs.id < rhs.id;
        });

    if (metadata.m_prototypes.empty())
    {
        throw std::runtime_error("Missing vegetation prototypes in vegetation tileset classification");
    }

    return metadata;
}

std::size_t VegetationTilesetMetadata::getPrototypeCount() const noexcept
{
    return m_prototypes.size();
}

const VegetationPrototype& VegetationTilesetMetadata::getPrototype(std::size_t index) const
{
    if (index >= m_prototypes.size())
    {
        throw std::runtime_error("Vegetation prototype index is out of range");
    }

    return m_prototypes[index];
}

const VegetationPrototype& VegetationTilesetMetadata::getPrototypeById(const std::string& id) const
{
    const auto prototypeIt = std::find_if(
        m_prototypes.begin(),
        m_prototypes.end(),
        [&id](const VegetationPrototype& prototype)
        {
            return prototype.id == id;
        });

    if (prototypeIt == m_prototypes.end())
    {
        throw std::runtime_error("Missing vegetation prototype in vegetation tileset metadata: " + id);
    }

    return *prototypeIt;
}

int VegetationTilesetMetadata::getMaxPrototypeWidthInTiles() const noexcept
{
    return m_maxPrototypeWidthInTiles;
}

int VegetationTilesetMetadata::getMaxPrototypeHeightInTiles() const noexcept
{
    return m_maxPrototypeHeightInTiles;
}

} // namespace detail
} // namespace rpg
