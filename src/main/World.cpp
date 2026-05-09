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

#include "WorldContent.hpp"
#include "WorldTerrainGenerator.hpp"

#include <cmath>
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
    const detail::TerrainGenerator terrainGenerator{m_state.config};
    m_state.spawnTile = terrainGenerator.generateSpawnTile();
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

ChunkCoordinates World::getChunkCoordinates(const TileCoordinates& coordinates) const noexcept
{
    return {
        detail::getChunkCoordinate(coordinates.x),
        detail::getChunkCoordinate(coordinates.y)};
}

ChunkMetadata World::getChunkMetadata(const ChunkCoordinates& coordinates) const
{
    return ensureChunkRetained(coordinates).metadata;
}

ChunkMetadata World::getChunkMetadata(const TileCoordinates& coordinates) const
{
    return getChunkMetadata(getChunkCoordinates(coordinates));
}

ChunkContent World::getChunkContent(const ChunkCoordinates& coordinates) const
{
    return ensureChunkRetained(coordinates).content;
}

ChunkContent World::getChunkContent(const TileCoordinates& coordinates) const
{
    return getChunkContent(getChunkCoordinates(coordinates));
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

std::vector<VisibleWorldContent> World::getVisibleContent(const ViewFrame& frame) const
{
    std::vector<VisibleWorldContent> visibleContent;

    const VisibleTileBounds bounds = getVisibleTileBounds(m_state.config.tileSize, frame);

    if (bounds.minX > bounds.maxX || bounds.minY > bounds.maxY)
    {
        return visibleContent;
    }

    const VisibleTileBounds contentBounds{
        bounds.minX - detail::getWorldContentVisibilityOverscanInTiles(),
        bounds.maxX + detail::getWorldContentVisibilityOverscanInTiles(),
        bounds.minY - detail::getWorldContentVisibilityOverscanInTiles(),
        bounds.maxY + detail::getWorldContentVisibilityOverscanInTiles()};
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
    std::size_t retainedCount = 0;

    for (const auto& [chunkKey, retainedChunk] : m_state.chunks)
    {
        (void)chunkKey;
        retainedCount += retainedChunk.content.instances.size();
    }

    return retainedCount;
}

World::State::RetainedChunkData& World::ensureChunkRetained(const ChunkCoordinates& coordinates) const
{
    const auto chunkKey = std::make_pair(coordinates.x, coordinates.y);
    const auto chunkIt = m_state.chunks.find(chunkKey);

    if (chunkIt != m_state.chunks.end())
    {
        return chunkIt->second;
    }

    const detail::TerrainGenerator terrainGenerator{m_state.config};
    detail::GeneratedChunkData chunkData = terrainGenerator.generateChunk(coordinates.x, coordinates.y);
    const detail::WorldContent worldContent{m_state.config};
    State::RetainedChunkData retainedChunk;
    retainedChunk.content = worldContent.generateChunkContent(coordinates, chunkData.metadata, chunkData.tiles);
    retainedChunk.tiles = std::move(chunkData.tiles);
    retainedChunk.metadata = std::move(chunkData.metadata);
    const auto insertedChunk = m_state.chunks.emplace(chunkKey, std::move(retainedChunk));
    return insertedChunk.first->second;
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
