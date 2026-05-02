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
#include "WorldTerrainGenerator.hpp"

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
    const ChunkCoordinates& chunkCoordinates,
    const ChunkMetadata& metadata,
    const ContentType type,
    const std::size_t ordinal) noexcept
{
    std::uint64_t recordId = worldSeed;
    recordId = mixRecordId(recordId, toUnsigned(chunkCoordinates.x));
    recordId = mixRecordId(recordId, toUnsigned(chunkCoordinates.y));
    recordId = mixRecordId(recordId, static_cast<std::uint64_t>(metadata.biomeSummary.dominantTileType));
    recordId = mixRecordId(recordId, static_cast<std::uint64_t>(metadata.traversabilitySummary.traversableTileCount));
    recordId = mixRecordId(recordId, static_cast<std::uint64_t>(metadata.traversabilitySummary.blockedTileCount));
    recordId = mixRecordId(recordId, static_cast<std::uint64_t>(type));
    return mixRecordId(recordId, ordinal);
}

[[nodiscard]] std::uint64_t makePlacementHash(
    const std::uint32_t worldSeed,
    const ChunkCoordinates& chunkCoordinates,
    const ChunkMetadata& metadata,
    const ContentType type) noexcept
{
    std::uint64_t placementHash = worldSeed;
    placementHash = mixRecordId(placementHash, toUnsigned(chunkCoordinates.x));
    placementHash = mixRecordId(placementHash, toUnsigned(chunkCoordinates.y));
    placementHash = mixRecordId(placementHash, static_cast<std::uint64_t>(metadata.biomeSummary.grassTileCount));
    placementHash = mixRecordId(placementHash, static_cast<std::uint64_t>(metadata.biomeSummary.forestTileCount));
    placementHash = mixRecordId(placementHash, static_cast<std::uint64_t>(metadata.biomeSummary.sandTileCount));
    placementHash = mixRecordId(placementHash, static_cast<std::uint64_t>(metadata.biomeSummary.waterTileCount));
    placementHash = mixRecordId(placementHash, static_cast<std::uint64_t>(metadata.traversabilitySummary.traversableTileCount));
    placementHash = mixRecordId(placementHash, static_cast<std::uint64_t>(metadata.traversabilitySummary.blockedTileCount));
    return mixRecordId(placementHash, static_cast<std::uint64_t>(type));
}

[[nodiscard]] TileCoordinates makeLocalContentTile(
    const std::uint64_t placementHash,
    const ContentType type) noexcept
{
    constexpr int kMarginInTiles = 2;
    constexpr int kUsableSpanInTiles = getChunkSizeInTiles() - (kMarginInTiles * 2);
    const int baseX = kMarginInTiles + static_cast<int>(placementHash % static_cast<std::uint64_t>(kUsableSpanInTiles));
    const int baseY = kMarginInTiles
        + static_cast<int>((placementHash >> 8U) % static_cast<std::uint64_t>(kUsableSpanInTiles));

    if (type == ContentType::SpawnSite)
    {
        return {baseX, baseY};
    }

    return {
        kMarginInTiles + ((baseX + (kUsableSpanInTiles / 2)) % kUsableSpanInTiles),
        kMarginInTiles + ((baseY + (kUsableSpanInTiles / 3)) % kUsableSpanInTiles)};
}

[[nodiscard]] WorldPosition getTileCenter(const TileCoordinates& coordinates, const float tileSize) noexcept
{
    return {
        (static_cast<float>(coordinates.x) + 0.5F) * tileSize,
        (static_cast<float>(coordinates.y) + 0.5F) * tileSize};
}

[[nodiscard]] ContentFootprint makeFootprint(const ContentType type, const float tileSize) noexcept
{
    if (type == ContentType::PointOfInterest)
    {
        return {{tileSize * 2.0F, tileSize * 2.0F}};
    }

    return {{tileSize, tileSize}};
}

[[nodiscard]] ContentAppearanceId makeAppearanceId(const std::uint64_t placementHash, const ContentType type) noexcept
{
    const std::uint64_t appearanceSeed = mixRecordId(placementHash, static_cast<std::uint64_t>(type));
    return {static_cast<std::uint32_t>((appearanceSeed & 0xFFFFFFFFULL) | 1ULL)};
}

[[nodiscard]] bool supportsSpawnSite(const ChunkMetadata& metadata) noexcept
{
    return metadata.traversabilitySummary.traversableTileCount > metadata.traversabilitySummary.blockedTileCount;
}

[[nodiscard]] bool supportsPointOfInterest(const ChunkMetadata& metadata) noexcept
{
    return metadata.traversabilitySummary.traversableTileCount > 0
        && (metadata.biomeSummary.dominantTileType == TileType::Sand
            || metadata.biomeSummary.dominantTileType == TileType::Forest);
}

void appendContentInstance(
    ChunkContent& content,
    const std::uint32_t worldSeed,
    const ChunkCoordinates& chunkCoordinates,
    const ChunkMetadata& metadata,
    const ContentType type,
    const float tileSize)
{
    const std::uint64_t placementHash = makePlacementHash(worldSeed, chunkCoordinates, metadata, type);
    const TileCoordinates localCoordinates = makeLocalContentTile(placementHash, type);
    const TileCoordinates worldTileCoordinates = getWorldTileCoordinates(
        chunkCoordinates.x,
        chunkCoordinates.y,
        localCoordinates);

    content.instances.push_back({
        makeRecordId(worldSeed, chunkCoordinates, metadata, type, content.instances.size()),
        type,
        getTileCenter(worldTileCoordinates, tileSize),
        makeFootprint(type, tileSize),
        makeAppearanceId(placementHash, type),
    });
}

} // namespace

WorldContent::WorldContent(const WorldConfig& config) noexcept
    : m_worldSeed(config.seed)
    , m_tileSize(config.tileSize)
{
}

ChunkContent WorldContent::generateChunkContent(
    const ChunkCoordinates& chunkCoordinates,
    const ChunkMetadata& metadata) const
{
    ChunkContent content;
    content.chunkCoordinates = chunkCoordinates;

    if (metadata.traversabilitySummary.traversableTileCount <= 0)
    {
        return content;
    }

    content.instances.reserve(2);

    if (supportsSpawnSite(metadata))
    {
        appendContentInstance(content, m_worldSeed, chunkCoordinates, metadata, ContentType::SpawnSite, m_tileSize);
    }

    if (supportsPointOfInterest(metadata))
    {
        appendContentInstance(
            content,
            m_worldSeed,
            chunkCoordinates,
            metadata,
            ContentType::PointOfInterest,
            m_tileSize);
    }

    return content;
}

} // namespace detail
} // namespace rpg
