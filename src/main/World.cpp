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

#include "WorldTerrainGenerator.hpp"

#include <algorithm>
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

[[nodiscard]] std::pair<int, int> toChunkKey(const TileCoordinates& coordinates) noexcept
{
    return {
        detail::getChunkCoordinate(coordinates.x),
        detail::getChunkCoordinate(coordinates.y)};
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

[[nodiscard]] VisibleTileBounds getVisibleTileBounds(const WorldConfig& config, const ViewFrame& frame) noexcept
{
    constexpr int kVisibleOverscanInTiles = 1;
    VisibleTileBounds bounds;

    if (config.widthInTiles <= 0 || config.heightInTiles <= 0 || config.tileSize <= 0.0F)
    {
        return bounds;
    }

    const float halfWidth = frame.size.width * 0.5F;
    const float halfHeight = frame.size.height * 0.5F;
    const float left = frame.center.x - halfWidth;
    const float right = frame.center.x + halfWidth;
    const float top = frame.center.y - halfHeight;
    const float bottom = frame.center.y + halfHeight;

    bounds.minX = std::max(
        0,
        static_cast<int>(std::floor(left / config.tileSize)) - kVisibleOverscanInTiles);
    bounds.maxX = std::min(
        config.widthInTiles - 1,
        static_cast<int>(std::ceil(right / config.tileSize)) - 1 + kVisibleOverscanInTiles);
    bounds.minY = std::max(
        0,
        static_cast<int>(std::floor(top / config.tileSize)) - kVisibleOverscanInTiles);
    bounds.maxY = std::min(
        config.heightInTiles - 1,
        static_cast<int>(std::ceil(bottom / config.tileSize)) - 1 + kVisibleOverscanInTiles);
    return bounds;
}

void populateChunkCache(
    const WorldConfig& config,
    std::map<std::pair<int, int>, std::vector<TileType>>& chunks)
{
    if (config.widthInTiles <= 0 || config.heightInTiles <= 0)
    {
        return;
    }

    const int lastChunkX = detail::getChunkCoordinate(config.widthInTiles - 1);
    const int lastChunkY = detail::getChunkCoordinate(config.heightInTiles - 1);

    for (int chunkY = 0; chunkY <= lastChunkY; ++chunkY)
    {
        for (int chunkX = 0; chunkX <= lastChunkX; ++chunkX)
        {
            detail::GeneratedChunkData chunkData = detail::generateChunkData(config, chunkX, chunkY);
            chunks.emplace(std::make_pair(chunkX, chunkY), std::move(chunkData.tiles));
        }
    }
}

} // namespace

World::World()
    : World(WorldConfig{})
{
}

World::World(const WorldConfig& config)
{
    m_state.config = config;
    populateChunkCache(m_state.config, m_state.chunks);

    const TileCoordinates center{m_state.config.widthInTiles / 2, m_state.config.heightInTiles / 2};

    for (int radius = 0; radius < std::max(m_state.config.widthInTiles, m_state.config.heightInTiles); ++radius)
    {
        const int minY = std::max(1, center.y - radius);
        const int maxY = std::min(m_state.config.heightInTiles - 2, center.y + radius);
        const int minX = std::max(1, center.x - radius);
        const int maxX = std::min(m_state.config.widthInTiles - 2, center.x + radius);

        for (int y = minY; y <= maxY; ++y)
        {
            for (int x = minX; x <= maxX; ++x)
            {
                const TileCoordinates coordinates{x, y};

                if (detail::isTraversableTileType(getTileType(coordinates)))
                {
                    m_state.spawnTile = coordinates;
                    return;
                }
            }
        }
    }

    m_state.spawnTile = center;
}

World::~World() = default;

int World::getWidthInTiles() const noexcept
{
    return m_state.config.widthInTiles;
}

int World::getHeightInTiles() const noexcept
{
    return m_state.config.heightInTiles;
}

float World::getTileSize() const noexcept
{
    return m_state.config.tileSize;
}

WorldSize World::getWorldSize() const noexcept
{
    return {
        static_cast<float>(m_state.config.widthInTiles) * m_state.config.tileSize,
        static_cast<float>(m_state.config.heightInTiles) * m_state.config.tileSize};
}

TileCoordinates World::getSpawnTile() const noexcept
{
    return m_state.spawnTile;
}

WorldPosition World::getSpawnPosition() const noexcept
{
    return getTileCenter(m_state.spawnTile);
}

bool World::isInBounds(const TileCoordinates& coordinates) const noexcept
{
    return coordinates.x >= 0
        && coordinates.x < m_state.config.widthInTiles
        && coordinates.y >= 0
        && coordinates.y < m_state.config.heightInTiles;
}

bool World::isTraversable(const TileCoordinates& coordinates) const noexcept
{
    return isInBounds(coordinates) && detail::isTraversableTileType(getTileType(coordinates));
}

bool World::isTraversable(const WorldPosition& position) const noexcept
{
    return isTraversable(getTileCoordinates(position));
}

TileType World::getTileType(const TileCoordinates& coordinates) const noexcept
{
    if (!isInBounds(coordinates))
    {
        return TileType::Water;
    }

    const auto chunkIt = m_state.chunks.find(toChunkKey(coordinates));

    if (chunkIt == m_state.chunks.end())
    {
        return TileType::Water;
    }

    return chunkIt->second[toIndex(toChunkLocalCoordinates(coordinates), detail::getChunkSizeInTiles())];
}

std::vector<VisibleWorldTile> World::getVisibleTiles(const ViewFrame& frame) const
{
    std::vector<VisibleWorldTile> visibleTiles;

    const VisibleTileBounds bounds = getVisibleTileBounds(m_state.config, frame);

    if (bounds.minX > bounds.maxX || bounds.minY > bounds.maxY)
    {
        return visibleTiles;
    }

    visibleTiles.reserve(static_cast<std::size_t>((bounds.maxX - bounds.minX + 1) * (bounds.maxY - bounds.minY + 1)));

    const int minChunkX = detail::getChunkCoordinate(bounds.minX);
    const int maxChunkX = detail::getChunkCoordinate(bounds.maxX);
    const int minChunkY = detail::getChunkCoordinate(bounds.minY);
    const int maxChunkY = detail::getChunkCoordinate(bounds.maxY);
    const int chunkSizeInTiles = detail::getChunkSizeInTiles();

    for (int chunkY = minChunkY; chunkY <= maxChunkY; ++chunkY)
    {
        for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX)
        {
            const auto chunkIt = m_state.chunks.find(std::make_pair(chunkX, chunkY));

            if (chunkIt == m_state.chunks.end())
            {
                continue;
            }

            for (int localY = 0; localY < chunkSizeInTiles; ++localY)
            {
                for (int localX = 0; localX < chunkSizeInTiles; ++localX)
                {
                    const TileCoordinates worldCoordinates = detail::getWorldTileCoordinates(
                        chunkX,
                        chunkY,
                        {localX, localY});

                    if (!isInBounds(worldCoordinates)
                        || worldCoordinates.x < bounds.minX
                        || worldCoordinates.x > bounds.maxX
                        || worldCoordinates.y < bounds.minY
                        || worldCoordinates.y > bounds.maxY)
                    {
                        continue;
                    }

                    visibleTiles.push_back({
                        worldCoordinates,
                        chunkIt->second[toIndex({localX, localY}, chunkSizeInTiles)],
                        getTileCenter(worldCoordinates)});
                }
            }
        }
    }

    return visibleTiles;
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
