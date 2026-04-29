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

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rpg
{

namespace
{

constexpr int kWorldWidthInTiles = 64;
constexpr int kWorldHeightInTiles = 64;
constexpr float kTileSize = 32.0F;
constexpr std::uint32_t kWorldSeed = 0x00C0FFEEU;

[[nodiscard]] std::size_t toIndex(const TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

[[nodiscard]] std::uint32_t hashCoordinates(const int x, const int y) noexcept
{
    std::uint32_t value = kWorldSeed;
    value ^= static_cast<std::uint32_t>(x * 73856093);
    value ^= static_cast<std::uint32_t>(y * 19349663);
    value ^= value >> 13U;
    value *= 1274126177U;
    value ^= value >> 16U;
    return value;
}

[[nodiscard]] TileType classifyTile(const int x, const int y) noexcept
{
    if (x == 0 || y == 0 || x == kWorldWidthInTiles - 1 || y == kWorldHeightInTiles - 1)
    {
        return TileType::Water;
    }

    const std::uint32_t noise = hashCoordinates(x, y) % 100U;

    if (noise < 12U)
    {
        return TileType::Water;
    }

    if (noise < 26U)
    {
        return TileType::Sand;
    }

    if (noise > 84U)
    {
        return TileType::Forest;
    }

    return TileType::Grass;
}

[[nodiscard]] bool isTraversableTileType(const TileType tileType) noexcept
{
    return tileType != TileType::Water;
}

[[nodiscard]] TileCoordinates findSpawnTile(
    const std::vector<TileType>& tiles,
    const int widthInTiles,
    const int heightInTiles) noexcept
{
    const TileCoordinates center{widthInTiles / 2, heightInTiles / 2};

    for (int radius = 0; radius < std::max(widthInTiles, heightInTiles); ++radius)
    {
        const int minY = std::max(1, center.y - radius);
        const int maxY = std::min(heightInTiles - 2, center.y + radius);
        const int minX = std::max(1, center.x - radius);
        const int maxX = std::min(widthInTiles - 2, center.x + radius);

        for (int y = minY; y <= maxY; ++y)
        {
            for (int x = minX; x <= maxX; ++x)
            {
                const TileCoordinates coordinates{x, y};

                if (isTraversableTileType(tiles[toIndex(coordinates, widthInTiles)]))
                {
                    return coordinates;
                }
            }
        }
    }

    return center;
}

} // namespace

World::World()
{
    m_state.widthInTiles = kWorldWidthInTiles;
    m_state.heightInTiles = kWorldHeightInTiles;
    m_state.tileSize = kTileSize;
    m_state.tiles.resize(static_cast<std::size_t>(m_state.widthInTiles * m_state.heightInTiles));

    for (int y = 0; y < m_state.heightInTiles; ++y)
    {
        for (int x = 0; x < m_state.widthInTiles; ++x)
        {
            const TileCoordinates coordinates{x, y};
            m_state.tiles[toIndex(coordinates, m_state.widthInTiles)] = classifyTile(x, y);
        }
    }

    m_state.spawnTile = findSpawnTile(m_state.tiles, m_state.widthInTiles, m_state.heightInTiles);
}

World::~World() = default;

int World::getWidthInTiles() const noexcept
{
    return m_state.widthInTiles;
}

int World::getHeightInTiles() const noexcept
{
    return m_state.heightInTiles;
}

float World::getTileSize() const noexcept
{
    return m_state.tileSize;
}

WorldSize World::getWorldSize() const noexcept
{
    return {
        static_cast<float>(m_state.widthInTiles) * m_state.tileSize,
        static_cast<float>(m_state.heightInTiles) * m_state.tileSize};
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
        && coordinates.x < m_state.widthInTiles
        && coordinates.y >= 0
        && coordinates.y < m_state.heightInTiles;
}

bool World::isTraversable(const TileCoordinates& coordinates) const noexcept
{
    return isInBounds(coordinates) && isTraversableTileType(getTileType(coordinates));
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

    return m_state.tiles[toIndex(coordinates, m_state.widthInTiles)];
}

WorldPosition World::getTileCenter(const TileCoordinates& coordinates) const noexcept
{
    return {
        (static_cast<float>(coordinates.x) + 0.5F) * m_state.tileSize,
        (static_cast<float>(coordinates.y) + 0.5F) * m_state.tileSize};
}

TileCoordinates World::getTileCoordinates(const WorldPosition& position) const noexcept
{
    return {
        static_cast<int>(std::floor(position.x / m_state.tileSize)),
        static_cast<int>(std::floor(position.y / m_state.tileSize))};
}

} // namespace rpg
