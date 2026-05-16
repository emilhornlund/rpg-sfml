/**
 * @file World.cpp
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

#include <main/World.hpp>

#include "RoadStampSupport.hpp"
#include "WorldContent.hpp"
#include "WorldTerrainGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

namespace rpg
{

namespace
{

[[nodiscard]] std::size_t toIndex(const TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

[[nodiscard]] TileCoordinates toChunkLocalCoordinates(const TileCoordinates& coordinates) noexcept
{
    return {
        detail::getChunkLocalCoordinate(coordinates.x),
        detail::getChunkLocalCoordinate(coordinates.y)};
}

struct VisibleTileBounds
{
    int minX = 0;
    int maxX = -1;
    int minY = 0;
    int maxY = -1;
};

struct VisibleChunkBounds
{
    int minX = 0;
    int maxX = -1;
    int minY = 0;
    int maxY = -1;
};

struct WorldBounds
{
    float left = 0.0F;
    float right = 0.0F;
    float top = 0.0F;
    float bottom = 0.0F;
};

[[nodiscard]] WorldBounds getWorldBounds(const ViewFrame& frame) noexcept
{
    const float halfWidth = frame.size.width * 0.5F;
    const float halfHeight = frame.size.height * 0.5F;
    return {
        frame.center.x - halfWidth,
        frame.center.x + halfWidth,
        frame.center.y - halfHeight,
        frame.center.y + halfHeight};
}

[[nodiscard]] VisibleTileBounds getVisibleTileBounds(const float tileSize, const ViewFrame& frame) noexcept
{
    constexpr int kVisibleOverscanInTiles = 1;
    VisibleTileBounds bounds;

    if (tileSize <= 0.0F || frame.size.width <= 0.0F || frame.size.height <= 0.0F)
    {
        return bounds;
    }

    const float halfWidth = frame.size.width * 0.5F;
    const float halfHeight = frame.size.height * 0.5F;
    const float left = frame.center.x - halfWidth;
    const float right = frame.center.x + halfWidth;
    const float top = frame.center.y - halfHeight;
    const float bottom = frame.center.y + halfHeight;

    bounds.minX = static_cast<int>(std::floor(left / tileSize)) - kVisibleOverscanInTiles;
    bounds.maxX = static_cast<int>(std::ceil(right / tileSize)) - 1 + kVisibleOverscanInTiles;
    bounds.minY = static_cast<int>(std::floor(top / tileSize)) - kVisibleOverscanInTiles;
    bounds.maxY = static_cast<int>(std::ceil(bottom / tileSize)) - 1 + kVisibleOverscanInTiles;
    return bounds;
}

[[nodiscard]] VisibleChunkBounds getVisibleChunkBounds(const VisibleTileBounds& bounds) noexcept
{
    return {
        detail::getChunkCoordinate(bounds.minX),
        detail::getChunkCoordinate(bounds.maxX),
        detail::getChunkCoordinate(bounds.minY),
        detail::getChunkCoordinate(bounds.maxY)};
}

[[nodiscard]] VisibleTileBounds getContentTileBounds(const float tileSize, const ViewFrame& frame) noexcept
{
    const VisibleTileBounds bounds = getVisibleTileBounds(tileSize, frame);

    if (bounds.minX > bounds.maxX || bounds.minY > bounds.maxY)
    {
        return bounds;
    }

    const int contentOverscanInTiles = detail::getWorldContentVisibilityOverscanInTiles();
    return {
        bounds.minX - contentOverscanInTiles,
        bounds.maxX + contentOverscanInTiles,
        bounds.minY - contentOverscanInTiles,
        bounds.maxY + contentOverscanInTiles};
}

[[nodiscard]] VisibleChunkBounds expandChunkBounds(const VisibleChunkBounds& bounds, const int marginInChunks) noexcept
{
    return {
        bounds.minX - marginInChunks,
        bounds.maxX + marginInChunks,
        bounds.minY - marginInChunks,
        bounds.maxY + marginInChunks};
}

[[nodiscard]] bool intersectsWorldBounds(const WorldBounds& bounds, const ContentInstance& instance) noexcept
{
    if (instance.footprint.size.width <= 0.0F || instance.footprint.size.height <= 0.0F)
    {
        return false;
    }

    const float left = instance.position.x + instance.footprint.offset.x;
    const float right = left + instance.footprint.size.width;
    const float top = instance.position.y + instance.footprint.offset.y;
    const float bottom = top + instance.footprint.size.height;

    return left <= bounds.right
        && right >= bounds.left
        && top <= bounds.bottom
        && bottom >= bounds.top;
}

} // namespace

World::World()
    : World(WorldConfig{})
{
}

World::World(const WorldConfig& config)
{
    m_state.config = config;
    m_state.terrainGenerator = std::make_shared<detail::TerrainGenerator>(m_state.config);
    m_state.spawnTile = m_state.terrainGenerator->generateSpawnTile();
    m_state.roadNetwork = std::make_shared<detail::RoadNetwork>(
        detail::buildRoadNetwork(m_state.spawnTile, m_state.config.seed));
    m_state.worldContent = std::make_shared<detail::WorldContent>(
        m_state.config,
        m_state.roadNetwork,
        m_state.terrainGenerator);
}

World::~World() = default;

float World::getTileSize() const noexcept
{
    return m_state.config.tileSize;
}

std::uint32_t World::getGenerationSeed() const noexcept
{
    return m_state.config.seed;
}

TileCoordinates World::getSpawnTile() const noexcept
{
    return m_state.spawnTile;
}

WorldPosition World::getSpawnPosition() const noexcept
{
    return getTileCenter(m_state.spawnTile);
}

bool World::isTraversable(const TileCoordinates& coordinates) const
{
    return detail::isTraversableTileType(getTileType(coordinates));
}

bool World::isTraversable(const WorldPosition& position) const
{
    return isTraversable(getTileCoordinates(position));
}

TileType World::getTileType(const TileCoordinates& coordinates) const
{
    const State::RetainedChunkData& chunk = ensureChunkRetained(getChunkCoordinates(coordinates));
    return chunk.tiles[toIndex(toChunkLocalCoordinates(coordinates), detail::getChunkSizeInTiles())];
}

bool World::hasRoadOverlay(const TileCoordinates& coordinates) const
{
    const auto getTileTypeAtCoordinates = [this](const TileCoordinates& tileCoordinates)
    {
        return getTileType(tileCoordinates);
    };
    return detail::getRoadStampedTile(
        coordinates,
        getTileType(coordinates),
        *m_state.roadNetwork,
        getTileTypeAtCoordinates).isPublished;
}

ChunkCoordinates World::getChunkCoordinates(const TileCoordinates& coordinates) const noexcept
{
    return {
        detail::getChunkCoordinate(coordinates.x),
        detail::getChunkCoordinate(coordinates.y)};
}

ChunkMetadata World::getChunkMetadata(const ChunkCoordinates& coordinates) const
{
    return getChunkMetadataRef(coordinates);
}

ChunkMetadata World::getChunkMetadata(const TileCoordinates& coordinates) const
{
    return getChunkMetadata(getChunkCoordinates(coordinates));
}

const ChunkMetadata& World::getChunkMetadataRef(const ChunkCoordinates& coordinates) const
{
    return ensureChunkRetained(coordinates).metadata;
}

const ChunkMetadata& World::getChunkMetadataRef(const TileCoordinates& coordinates) const
{
    return getChunkMetadataRef(getChunkCoordinates(coordinates));
}

ChunkContent World::getChunkContent(const ChunkCoordinates& coordinates) const
{
    return getChunkContentRef(coordinates);
}

ChunkContent World::getChunkContent(const TileCoordinates& coordinates) const
{
    return getChunkContent(getChunkCoordinates(coordinates));
}

const ChunkContent& World::getChunkContentRef(const ChunkCoordinates& coordinates) const
{
    return ensureChunkRetained(coordinates).content;
}

const ChunkContent& World::getChunkContentRef(const TileCoordinates& coordinates) const
{
    return getChunkContentRef(getChunkCoordinates(coordinates));
}

std::vector<VisibleWorldTile> World::getVisibleTiles(const ViewFrame& frame) const
{
    std::vector<VisibleWorldTile> visibleTiles;

    const VisibleTileBounds bounds = getVisibleTileBounds(m_state.config.tileSize, frame);

    if (bounds.minX > bounds.maxX || bounds.minY > bounds.maxY)
    {
        return visibleTiles;
    }

    visibleTiles.reserve(static_cast<std::size_t>((bounds.maxX - bounds.minX + 1) * (bounds.maxY - bounds.minY + 1)));

    const VisibleChunkBounds chunkBounds = getVisibleChunkBounds(bounds);
    const int chunkSizeInTiles = detail::getChunkSizeInTiles();

    for (int chunkY = chunkBounds.minY; chunkY <= chunkBounds.maxY; ++chunkY)
    {
        for (int chunkX = chunkBounds.minX; chunkX <= chunkBounds.maxX; ++chunkX)
        {
            const State::RetainedChunkData& chunk = ensureChunkRetained({chunkX, chunkY});

            for (int localY = 0; localY < chunkSizeInTiles; ++localY)
            {
                for (int localX = 0; localX < chunkSizeInTiles; ++localX)
                {
                    const TileCoordinates worldCoordinates = detail::getWorldTileCoordinates(
                        chunkX,
                        chunkY,
                        {localX, localY});

                    if (worldCoordinates.x < bounds.minX
                        || worldCoordinates.x > bounds.maxX
                        || worldCoordinates.y < bounds.minY
                        || worldCoordinates.y > bounds.maxY)
                    {
                        continue;
                    }

                    visibleTiles.push_back({
                        worldCoordinates,
                        chunk.tiles[toIndex({localX, localY}, chunkSizeInTiles)],
                        getTileCenter(worldCoordinates)});
                }
            }
        }
    }

    return visibleTiles;
}

std::vector<VisibleWorldRoadOverlay> World::getVisibleRoadOverlays(const ViewFrame& frame) const
{
    std::vector<VisibleWorldRoadOverlay> visibleRoadOverlays;

    const VisibleTileBounds bounds = getVisibleTileBounds(m_state.config.tileSize, frame);

    if (bounds.minX > bounds.maxX || bounds.minY > bounds.maxY)
    {
        return visibleRoadOverlays;
    }

    const VisibleChunkBounds chunkBounds = getVisibleChunkBounds(bounds);
    const int chunkSizeInTiles = detail::getChunkSizeInTiles();

    for (int chunkY = chunkBounds.minY; chunkY <= chunkBounds.maxY; ++chunkY)
    {
        for (int chunkX = chunkBounds.minX; chunkX <= chunkBounds.maxX; ++chunkX)
        {
            const State::RetainedChunkData& chunk = ensureChunkRetained({chunkX, chunkY});

            for (int localY = 0; localY < chunkSizeInTiles; ++localY)
            {
                for (int localX = 0; localX < chunkSizeInTiles; ++localX)
                {
                    const TileCoordinates worldCoordinates = detail::getWorldTileCoordinates(
                        chunkX,
                        chunkY,
                        {localX, localY});

                    if (worldCoordinates.x < bounds.minX
                        || worldCoordinates.x > bounds.maxX
                        || worldCoordinates.y < bounds.minY
                        || worldCoordinates.y > bounds.maxY)
                    {
                        continue;
                    }

                    const TileType tileType = chunk.tiles[toIndex({localX, localY}, chunkSizeInTiles)];

                    const auto getTileTypeAtCoordinates = [this](const TileCoordinates& tileCoordinates)
                    {
                        return getTileType(tileCoordinates);
                    };

                    const detail::RoadStampedTile stampedTile = detail::getRoadStampedTile(
                        worldCoordinates,
                        tileType,
                        *m_state.roadNetwork,
                        getTileTypeAtCoordinates);

                    if (!stampedTile.isPublished)
                    {
                        continue;
                    }

                    visibleRoadOverlays.push_back(
                        {worldCoordinates,
                         tileType,
                         getTileCenter(worldCoordinates),
                         true,
                         stampedTile.publishedNeighborOccupancy});
                }
            }
        }
    }

    return visibleRoadOverlays;
}

std::vector<VisibleWorldContent> World::getVisibleContent(const ViewFrame& frame) const
{
    std::vector<VisibleWorldContent> visibleContent;

    const VisibleTileBounds bounds = getVisibleTileBounds(m_state.config.tileSize, frame);

    if (bounds.minX > bounds.maxX || bounds.minY > bounds.maxY)
    {
        return visibleContent;
    }

    const VisibleTileBounds contentBounds = getContentTileBounds(m_state.config.tileSize, frame);
    const VisibleChunkBounds contentChunkBounds = getVisibleChunkBounds(contentBounds);
    const WorldBounds worldBounds = getWorldBounds(frame);

    for (int chunkY = contentChunkBounds.minY; chunkY <= contentChunkBounds.maxY; ++chunkY)
    {
        for (int chunkX = contentChunkBounds.minX; chunkX <= contentChunkBounds.maxX; ++chunkX)
        {
            const State::RetainedChunkData& chunk = ensureChunkRetained({chunkX, chunkY});

            for (const ContentInstance& instance : chunk.content.instances)
            {
                if (intersectsWorldBounds(worldBounds, instance))
                {
                    visibleContent.push_back({instance});
                }
            }
        }
    }

    return visibleContent;
}

std::size_t World::getRetainedGeneratedContentCount() const noexcept
{
    return m_state.retainedGeneratedContentCount;
}

std::size_t World::getRetainedChunkCount() const noexcept
{
    return m_state.chunks.size();
}

void World::updateRetentionWindow(const ViewFrame& frame)
{
    constexpr int kRetentionMarginInChunks = 1;
    const VisibleTileBounds contentBounds = getContentTileBounds(m_state.config.tileSize, frame);

    if (contentBounds.minX > contentBounds.maxX || contentBounds.minY > contentBounds.maxY)
    {
        return;
    }

    const VisibleChunkBounds retentionChunkBounds = expandChunkBounds(
        getVisibleChunkBounds(contentBounds),
        kRetentionMarginInChunks);
    m_state.retentionWindow = {
        retentionChunkBounds.minX,
        retentionChunkBounds.maxX,
        retentionChunkBounds.minY,
        retentionChunkBounds.maxY,
        true};
    pruneRetainedChunks();
}

World::State::RetainedChunkData& World::ensureChunkRetained(const ChunkCoordinates& coordinates) const
{
    const auto chunkKey = std::make_pair(coordinates.x, coordinates.y);
    const auto chunkIt = m_state.chunks.find(chunkKey);

    if (chunkIt != m_state.chunks.end())
    {
        return chunkIt->second;
    }

    detail::GeneratedChunkData chunkData = m_state.terrainGenerator->generateChunk(coordinates.x, coordinates.y);
    State::RetainedChunkData retainedChunk;
    retainedChunk.content = m_state.worldContent->generateChunkContent(coordinates, chunkData.metadata, chunkData.tiles);
    retainedChunk.tiles = std::move(chunkData.tiles);
    retainedChunk.metadata = std::move(chunkData.metadata);
    const auto insertedChunk = m_state.chunks.emplace(chunkKey, std::move(retainedChunk));
    m_state.retainedGeneratedContentCount += insertedChunk.first->second.content.instances.size();
    return insertedChunk.first->second;
}

void World::pruneRetainedChunks() const
{
    if (!m_state.retentionWindow.isActive)
    {
        return;
    }

    auto chunkIt = m_state.chunks.begin();

    while (chunkIt != m_state.chunks.end())
    {
        const ChunkCoordinates coordinates{chunkIt->first.first, chunkIt->first.second};

        if (coordinates.x >= m_state.retentionWindow.minChunkX
            && coordinates.x <= m_state.retentionWindow.maxChunkX
            && coordinates.y >= m_state.retentionWindow.minChunkY
            && coordinates.y <= m_state.retentionWindow.maxChunkY)
        {
            ++chunkIt;
            continue;
        }

        m_state.retainedGeneratedContentCount -= chunkIt->second.content.instances.size();
        chunkIt = m_state.chunks.erase(chunkIt);
    }
}

WorldPosition World::getTileCenter(const TileCoordinates& coordinates) const noexcept
{
    return {
        (static_cast<float>(coordinates.x) + 0.5F) * m_state.config.tileSize,
        (static_cast<float>(coordinates.y) + 0.5F) * m_state.config.tileSize};
}

TileCoordinates World::getTileCoordinates(const WorldPosition& position) const noexcept
{
    return {
        static_cast<int>(std::floor(position.x / m_state.config.tileSize)),
        static_cast<int>(std::floor(position.y / m_state.config.tileSize))};
}

} // namespace rpg
