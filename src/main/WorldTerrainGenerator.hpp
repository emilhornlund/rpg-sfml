/**
 * @file WorldTerrainGenerator.hpp
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

#ifndef RPG_MAIN_WORLD_TERRAIN_GENERATOR_HPP
#define RPG_MAIN_WORLD_TERRAIN_GENERATOR_HPP

#include <main/World.hpp>

#include <cstddef>
#include <vector>

namespace rpg
{
namespace detail
{

struct GeneratedWorldData
{
    TileCoordinates spawnTile{0, 0};
    std::vector<TileType> tiles;
};

struct GeneratedChunkData
{
    int chunkX = 0;
    int chunkY = 0;
    std::vector<TileType> tiles;
    ChunkMetadata metadata;
};

class TerrainGenerator
{
public:
    explicit TerrainGenerator(const WorldConfig& config) noexcept;

    [[nodiscard]] GeneratedChunkData generateChunk(int chunkX, int chunkY) const;

    [[nodiscard]] GeneratedWorldData generateWorld() const;

    [[nodiscard]] TileCoordinates generateSpawnTile() const;

private:
    WorldConfig m_config;
};

[[nodiscard]] constexpr int getChunkSizeInTiles() noexcept
{
    return 16;
}

[[nodiscard]] int getChunkCoordinate(int tileCoordinate) noexcept;

[[nodiscard]] int getChunkLocalCoordinate(int tileCoordinate) noexcept;

[[nodiscard]] TileCoordinates getWorldTileCoordinates(
    int chunkX,
    int chunkY,
    const TileCoordinates& localCoordinates) noexcept;

[[nodiscard]] std::size_t getGeneratedChunkCount() noexcept;

void resetGeneratedChunkCount() noexcept;

[[nodiscard]] std::size_t getTerrainGeneratorConstructionCount() noexcept;

void resetTerrainGeneratorConstructionCount() noexcept;

[[nodiscard]] bool isTraversableTileType(TileType tileType) noexcept;

} // namespace detail
} // namespace rpg

#endif // RPG_MAIN_WORLD_TERRAIN_GENERATOR_HPP
