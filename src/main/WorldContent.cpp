/**
 * @file WorldContent.cpp
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

#include "WorldContent.hpp"
#include "GameAssetSupport.hpp"
#include "WorldTerrainGenerator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace rpg
{
namespace detail
{

namespace
{

[[nodiscard]] std::uint64_t mixRecordId(std::uint64_t seed, const std::uint64_t value) noexcept
{
    constexpr std::uint64_t kGoldenRatio = 0x9E3779B97F4A7C15ULL;
    seed ^= value + kGoldenRatio + (seed << 6U) + (seed >> 2U);
    return seed;
}

[[nodiscard]] std::uint64_t toUnsigned(const int value) noexcept
{
    return static_cast<std::uint64_t>(static_cast<std::uint32_t>(value));
}

[[nodiscard]] std::uint64_t makeRecordId(
    const std::uint32_t worldSeed,
    const TileCoordinates& anchorTile,
    const ContentType type,
    const std::string_view prototypeId) noexcept
{
    constexpr std::uint64_t kFnvOffsetBasis = 14695981039346656037ULL;
    constexpr std::uint64_t kFnvPrime = 1099511628211ULL;
    auto appendByte = [](std::uint64_t hashValue, const std::uint8_t byteValue) noexcept
    {
        return (hashValue ^ byteValue) * kFnvPrime;
    };
    auto appendU32 = [&appendByte](std::uint64_t hashValue, const std::uint32_t value) noexcept
    {
        std::uint64_t currentHash = hashValue;

        for (int shift = 0; shift < 32; shift += 8)
        {
            currentHash = appendByte(currentHash, static_cast<std::uint8_t>((value >> shift) & 0xFFU));
        }

        return currentHash;
    };

    std::uint64_t recordId = kFnvOffsetBasis;
    recordId = appendU32(recordId, worldSeed);
    recordId = appendU32(recordId, static_cast<std::uint32_t>(anchorTile.x));
    recordId = appendU32(recordId, static_cast<std::uint32_t>(anchorTile.y));
    recordId = appendByte(recordId, static_cast<std::uint8_t>(type));

    for (const char character : prototypeId)
    {
        recordId = appendByte(recordId, static_cast<std::uint8_t>(character));
    }

    return recordId;
}

[[nodiscard]] std::uint32_t hashCoordinates(const std::uint32_t seed, const int x, const int y) noexcept
{
    std::uint32_t value = seed;
    value ^= static_cast<std::uint32_t>(x * 73856093);
    value ^= static_cast<std::uint32_t>(y * 19349663);
    value ^= value >> 13U;
    value *= 1274126177U;
    value ^= value >> 16U;
    return value;
}

[[nodiscard]] float toUnitFloat(const std::uint32_t seed, const int x, const int y) noexcept
{
    return static_cast<float>(hashCoordinates(seed, x, y))
        / static_cast<float>(std::numeric_limits<std::uint32_t>::max());
}

[[nodiscard]] float smoothstep(const float value) noexcept
{
    const float clampedValue = std::clamp(value, 0.0F, 1.0F);
    return clampedValue * clampedValue * (3.0F - (2.0F * clampedValue));
}

[[nodiscard]] float interpolate(const float start, const float end, const float factor) noexcept
{
    return start + ((end - start) * factor);
}

[[nodiscard]] float sampleValueNoise(const std::uint32_t seed, const float sampleX, const float sampleY) noexcept
{
    const int originX = static_cast<int>(std::floor(sampleX));
    const int originY = static_cast<int>(std::floor(sampleY));
    const float fractionX = smoothstep(sampleX - static_cast<float>(originX));
    const float fractionY = smoothstep(sampleY - static_cast<float>(originY));

    const float topLeft = toUnitFloat(seed, originX, originY);
    const float topRight = toUnitFloat(seed, originX + 1, originY);
    const float bottomLeft = toUnitFloat(seed, originX, originY + 1);
    const float bottomRight = toUnitFloat(seed, originX + 1, originY + 1);

    const float top = interpolate(topLeft, topRight, fractionX);
    const float bottom = interpolate(bottomLeft, bottomRight, fractionX);
    return interpolate(top, bottom, fractionY);
}

[[nodiscard]] WorldPosition getTileCenter(const TileCoordinates& coordinates, const float tileSize) noexcept
{
    return {
        (static_cast<float>(coordinates.x) + 0.5F) * tileSize,
        (static_cast<float>(coordinates.y) + 0.5F) * tileSize};
}

struct PrototypeDescriptor
{
    std::string_view id;
    ContentType type = ContentType::Tree;
    int weight = 1;
};

constexpr int kTreePlacementCellSizeInTiles = 4;
constexpr std::uint32_t kForestTreeSalt = 0x41C64E6DU;
constexpr std::uint32_t kGrassTreeSalt = 0x9E3779B9U;
constexpr std::uint32_t kForestShrubSalt = 0x7F4A7C15U;
constexpr std::uint32_t kGrassShrubSalt = 0x63D83595U;

constexpr std::array<PrototypeDescriptor, 5> kForestTreePrototypes = {{
    {"oak_tree_large_dark_1", ContentType::Tree, 4},
    {"oak_tree_large_yellow_1", ContentType::Tree, 3},
    {"pine_tree_medium_1", ContentType::Tree, 3},
    {"round_tree_medium_1", ContentType::Tree, 2},
    {"dead_tree_large_1", ContentType::Tree, 1},
}};

constexpr std::array<PrototypeDescriptor, 3> kGrassTreePrototypes = {{
    {"round_tree_small_1", ContentType::Tree, 3},
    {"pine_tree_small_1", ContentType::Tree, 2},
    {"flowering_tree_pink_small_1", ContentType::Tree, 1},
}};

constexpr std::array<PrototypeDescriptor, 4> kForestShrubPrototypes = {{
    {"bush_large_1", ContentType::Shrub, 4},
    {"bush_small_1", ContentType::Shrub, 5},
    {"stump_cut_small_1", ContentType::Shrub, 2},
    {"fallen_log_large_1", ContentType::Shrub, 1},
}};

constexpr std::array<PrototypeDescriptor, 2> kGrassShrubPrototypes = {{
    {"bush_small_1", ContentType::Shrub, 4},
    {"flowering_tree_pink_small_1", ContentType::Tree, 1},
}};

} // namespace

namespace
{

[[nodiscard]] const VegetationTilesetMetadata& getVegetationPlacementMetadata()
{
#ifndef RPG_DEFAULT_ASSET_ROOT_PATH
#error "RPG_DEFAULT_ASSET_ROOT_PATH must be defined for vegetation placement metadata loading."
#endif
    static const VegetationTilesetMetadata metadata =
        loadVegetationTilesetMetadata(std::filesystem::path(RPG_DEFAULT_ASSET_ROOT_PATH));
    return metadata;
}

[[nodiscard]] const VegetationPrototype& getVegetationPrototype(const std::string_view prototypeId)
{
    return getVegetationPlacementMetadata().getPrototypeById(std::string(prototypeId));
}

[[nodiscard]] constexpr int floorDivide(const int value, const int divisor) noexcept
{
    return value >= 0 ? value / divisor : -(((-value) + divisor - 1) / divisor);
}

[[nodiscard]] constexpr int positiveModulo(const int value, const int divisor) noexcept
{
    const int remainder = value % divisor;
    return remainder < 0 ? remainder + divisor : remainder;
}

[[nodiscard]] const TileType& getChunkTile(
    const std::vector<TileType>& tiles,
    const ChunkCoordinates& chunkCoordinates,
    const TileCoordinates& worldTile) noexcept
{
    const TileCoordinates localCoordinates{
        worldTile.x - chunkCoordinates.x * getChunkSizeInTiles(),
        worldTile.y - chunkCoordinates.y * getChunkSizeInTiles()};
    return tiles[static_cast<std::size_t>(localCoordinates.y * getChunkSizeInTiles() + localCoordinates.x)];
}

template <std::size_t N>
[[nodiscard]] const PrototypeDescriptor& choosePrototype(
    const std::array<PrototypeDescriptor, N>& descriptors,
    const std::uint32_t hashValue) noexcept
{
    int totalWeight = 0;

    for (const PrototypeDescriptor& descriptor : descriptors)
    {
        totalWeight += descriptor.weight;
    }

    int selection = static_cast<int>(hashValue % static_cast<std::uint32_t>(totalWeight));

    for (const PrototypeDescriptor& descriptor : descriptors)
    {
        if (selection < descriptor.weight)
        {
            return descriptor;
        }

        selection -= descriptor.weight;
    }

    return descriptors.front();
}

[[nodiscard]] ContentFootprint makeFootprint(const PrototypeDescriptor& descriptor, const float tileSize) noexcept
{
    const VegetationPrototype& prototype = getVegetationPrototype(descriptor.id);
    const int widthInTiles = prototype.bounds.maxOffsetX - prototype.bounds.minOffsetX + 1;
    const int heightInTiles = prototype.bounds.maxOffsetY - prototype.bounds.minOffsetY + 1;
    return {
        {
            (static_cast<float>(prototype.bounds.minOffsetX) - 0.5F) * tileSize,
            (static_cast<float>(prototype.bounds.minOffsetY) - 0.5F) * tileSize,
        },
        {
            static_cast<float>(widthInTiles) * tileSize,
            static_cast<float>(heightInTiles) * tileSize,
        }};
}

[[nodiscard]] ContentAppearanceId makeAppearanceId(
    const std::uint32_t worldSeed,
    const TileCoordinates& anchorTile,
    const std::string_view prototypeId) noexcept
{
    std::uint64_t appearanceSeed = worldSeed;
    appearanceSeed = mixRecordId(appearanceSeed, toUnsigned(anchorTile.x));
    appearanceSeed = mixRecordId(appearanceSeed, toUnsigned(anchorTile.y));

    for (const char character : prototypeId)
    {
        appearanceSeed = mixRecordId(appearanceSeed, static_cast<std::uint64_t>(static_cast<unsigned char>(character)));
    }

    return {static_cast<std::uint32_t>((appearanceSeed & 0xFFFFFFFFULL) | 1ULL)};
}

[[nodiscard]] float getTreePlacementChance(
    const std::uint32_t worldSeed,
    const TileCoordinates& anchorTile,
    const TileType tileType) noexcept
{
    const float clusterNoise = sampleValueNoise(
        worldSeed ^ 0xA511E9B3U,
        static_cast<float>(anchorTile.x) / 18.0F,
        static_cast<float>(anchorTile.y) / 18.0F);
    const float localNoise = sampleValueNoise(
        worldSeed ^ 0xC2B2AE35U,
        static_cast<float>(anchorTile.x) / 6.0F,
        static_cast<float>(anchorTile.y) / 6.0F);

    if (tileType == TileType::Forest)
    {
        return std::clamp(0.35F + (0.40F * clusterNoise) + (0.15F * localNoise), 0.0F, 0.95F);
    }

    return std::clamp(0.02F + (0.05F * clusterNoise) + (0.02F * localNoise), 0.0F, 0.20F);
}

[[nodiscard]] float getShrubPlacementChance(
    const std::uint32_t worldSeed,
    const TileCoordinates& anchorTile,
    const TileType tileType) noexcept
{
    const float clusterNoise = sampleValueNoise(
        worldSeed ^ 0x9E3779B9U,
        static_cast<float>(anchorTile.x) / 10.0F,
        static_cast<float>(anchorTile.y) / 10.0F);
    const float localNoise = sampleValueNoise(
        worldSeed ^ 0x7F4A7C15U,
        static_cast<float>(anchorTile.x) / 3.0F,
        static_cast<float>(anchorTile.y) / 3.0F);

    if (tileType == TileType::Forest)
    {
        return std::clamp(0.03F + (0.05F * clusterNoise) + (0.03F * localNoise), 0.0F, 0.25F);
    }

    return std::clamp(0.004F + (0.01F * clusterNoise) + (0.006F * localNoise), 0.0F, 0.08F);
}

void appendContentInstance(
    ChunkContent& content,
    const std::uint32_t worldSeed,
    const TileCoordinates& anchorTile,
    const PrototypeDescriptor& descriptor,
    const float tileSize)
{
    const WorldPosition anchorPosition = getTileCenter(anchorTile, tileSize);
    content.instances.push_back({
        makeRecordId(worldSeed, anchorTile, descriptor.type, descriptor.id),
        descriptor.type,
        std::string(descriptor.id),
        anchorTile,
        anchorPosition,
        makeFootprint(descriptor, tileSize),
        makeAppearanceId(worldSeed, anchorTile, descriptor.id),
        anchorPosition.y,
    });
}

[[nodiscard]] bool isTreeAnchorOccupied(
    const std::vector<TileCoordinates>& occupiedTreeAnchors,
    const TileCoordinates& candidate) noexcept
{
    return std::find_if(
               occupiedTreeAnchors.begin(),
               occupiedTreeAnchors.end(),
               [&candidate](const TileCoordinates& occupied)
               {
                   return occupied.x == candidate.x && occupied.y == candidate.y;
               })
        != occupiedTreeAnchors.end();
}

} // namespace

int getWorldContentVisibilityOverscanInTiles()
{
    const VegetationTilesetMetadata& metadata = getVegetationPlacementMetadata();
    return std::max(metadata.getMaxPrototypeWidthInTiles(), metadata.getMaxPrototypeHeightInTiles());
}

WorldContent::WorldContent(const WorldConfig& config) noexcept
    : m_worldSeed(config.seed)
    , m_tileSize(config.tileSize)
{
}

ChunkContent WorldContent::generateChunkContent(
    const ChunkCoordinates& chunkCoordinates,
    const ChunkMetadata& metadata,
    const std::vector<TileType>& tiles) const
{
    ChunkContent content;
    content.chunkCoordinates = chunkCoordinates;

    if (metadata.traversabilitySummary.traversableTileCount <= 0 || tiles.empty())
    {
        return content;
    }

    std::vector<TileCoordinates> occupiedTreeAnchors;
    const int chunkMinX = chunkCoordinates.x * getChunkSizeInTiles();
    const int chunkMinY = chunkCoordinates.y * getChunkSizeInTiles();
    const int chunkMaxX = chunkMinX + getChunkSizeInTiles() - 1;
    const int chunkMaxY = chunkMinY + getChunkSizeInTiles() - 1;
    const int minCellX = floorDivide(chunkMinX, kTreePlacementCellSizeInTiles);
    const int maxCellX = floorDivide(chunkMaxX, kTreePlacementCellSizeInTiles);
    const int minCellY = floorDivide(chunkMinY, kTreePlacementCellSizeInTiles);
    const int maxCellY = floorDivide(chunkMaxY, kTreePlacementCellSizeInTiles);

    for (int cellY = minCellY; cellY <= maxCellY; ++cellY)
    {
        for (int cellX = minCellX; cellX <= maxCellX; ++cellX)
        {
            const std::uint32_t candidateHash = hashCoordinates(m_worldSeed ^ kForestTreeSalt, cellX, cellY);
            const TileCoordinates anchorTile{
                cellX * kTreePlacementCellSizeInTiles
                    + positiveModulo(static_cast<int>(candidateHash & 0xFFFFU), kTreePlacementCellSizeInTiles),
                cellY * kTreePlacementCellSizeInTiles
                    + positiveModulo(static_cast<int>((candidateHash >> 16U) & 0xFFFFU), kTreePlacementCellSizeInTiles)};

            if (anchorTile.x < chunkMinX || anchorTile.x > chunkMaxX || anchorTile.y < chunkMinY || anchorTile.y > chunkMaxY)
            {
                continue;
            }

            const TileType tileType = getChunkTile(tiles, chunkCoordinates, anchorTile);

            if (tileType != TileType::Forest && tileType != TileType::Grass)
            {
                continue;
            }

            const float chance = getTreePlacementChance(m_worldSeed, anchorTile, tileType);
            const float placementRoll = toUnitFloat(
                m_worldSeed ^ (tileType == TileType::Forest ? kForestTreeSalt : kGrassTreeSalt),
                anchorTile.x,
                anchorTile.y);

            if (placementRoll > chance)
            {
                continue;
            }

            const PrototypeDescriptor& descriptor = tileType == TileType::Forest
                ? choosePrototype(kForestTreePrototypes, candidateHash)
                : choosePrototype(kGrassTreePrototypes, candidateHash);
            appendContentInstance(content, m_worldSeed, anchorTile, descriptor, m_tileSize);
            occupiedTreeAnchors.push_back(anchorTile);
        }
    }

    for (int localY = 0; localY < getChunkSizeInTiles(); ++localY)
    {
        for (int localX = 0; localX < getChunkSizeInTiles(); ++localX)
        {
            const TileCoordinates anchorTile = getWorldTileCoordinates(chunkCoordinates.x, chunkCoordinates.y, {localX, localY});
            const TileType tileType = tiles[static_cast<std::size_t>(localY * getChunkSizeInTiles() + localX)];

            if ((tileType != TileType::Forest && tileType != TileType::Grass) || isTreeAnchorOccupied(occupiedTreeAnchors, anchorTile))
            {
                continue;
            }

            const float chance = getShrubPlacementChance(m_worldSeed, anchorTile, tileType);
            const float placementRoll = toUnitFloat(
                m_worldSeed ^ (tileType == TileType::Forest ? kForestShrubSalt : kGrassShrubSalt),
                anchorTile.x,
                anchorTile.y);

            if (placementRoll > chance)
            {
                continue;
            }

            const std::uint32_t shrubHash = hashCoordinates(m_worldSeed ^ kForestShrubSalt, anchorTile.x, anchorTile.y);
            const PrototypeDescriptor& descriptor = tileType == TileType::Forest
                ? choosePrototype(kForestShrubPrototypes, shrubHash)
                : choosePrototype(kGrassShrubPrototypes, shrubHash);
            appendContentInstance(content, m_worldSeed, anchorTile, descriptor, m_tileSize);
        }
    }

    std::sort(
        content.instances.begin(),
        content.instances.end(),
        [](const ContentInstance& lhs, const ContentInstance& rhs)
        {
            if (lhs.anchorTile.y != rhs.anchorTile.y)
            {
                return lhs.anchorTile.y < rhs.anchorTile.y;
            }

            if (lhs.anchorTile.x != rhs.anchorTile.x)
            {
                return lhs.anchorTile.x < rhs.anchorTile.x;
            }

            return lhs.id < rhs.id;
        });

    return content;
}

} // namespace detail
} // namespace rpg
