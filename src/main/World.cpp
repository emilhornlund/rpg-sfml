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

#include <cmath>

namespace rpg
{

namespace
{

[[nodiscard]] std::size_t toIndex(const TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

} // namespace

World::World()
    : World(WorldConfig{})
{
}

World::World(const WorldConfig& config)
{
    m_state.config = config;
    const detail::GeneratedWorldData worldData = detail::generateWorldData(m_state.config);
    m_state.spawnTile = worldData.spawnTile;
    m_state.tiles = worldData.tiles;
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

    return m_state.tiles[toIndex(coordinates, m_state.config.widthInTiles)];
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
