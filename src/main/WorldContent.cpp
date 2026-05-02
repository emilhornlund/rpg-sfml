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
    const WorldContentType type,
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

void appendContentRecord(
    std::vector<WorldContentRecord>& records,
    const std::uint32_t worldSeed,
    const ChunkCoordinates& chunkCoordinates,
    const ChunkMetadata& metadata,
    const WorldContentType type)
{
    records.push_back({
        makeRecordId(worldSeed, chunkCoordinates, metadata, type, records.size()),
        chunkCoordinates,
        type,
    });
}

} // namespace

WorldContent::WorldContent(const std::uint32_t worldSeed) noexcept
    : m_worldSeed(worldSeed)
{
}

std::vector<WorldContentRecord> WorldContent::generateChunkContent(
    const ChunkCoordinates& chunkCoordinates,
    const ChunkMetadata& metadata) const
{
    std::vector<WorldContentRecord> records;

    if (metadata.traversabilitySummary.traversableTileCount <= 0)
    {
        return records;
    }

    records.reserve(2);

    if (supportsSpawnSite(metadata))
    {
        appendContentRecord(records, m_worldSeed, chunkCoordinates, metadata, WorldContentType::SpawnSite);
    }

    if (supportsPointOfInterest(metadata))
    {
        appendContentRecord(records, m_worldSeed, chunkCoordinates, metadata, WorldContentType::PointOfInterest);
    }

    return records;
}

} // namespace detail
} // namespace rpg
