/**
 * @file OverworldVerticalSliceTests.cpp
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

#include <main/Camera.hpp>
#include <main/Player.hpp>
#include <main/World.hpp>

#include <cmath>
#include <optional>

namespace
{

constexpr float kFloatTolerance = 0.001F;

[[nodiscard]] bool areClose(const float lhs, const float rhs) noexcept
{
    return std::fabs(lhs - rhs) < kFloatTolerance;
}

[[nodiscard]] std::optional<rpg::TileCoordinates> findAdjacentTile(
    const rpg::World& world,
    const rpg::TileCoordinates& origin,
    const bool wantTraversable)
{
    constexpr rpg::TileCoordinates kOffsets[] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}};

    for (const rpg::TileCoordinates& offset : kOffsets)
    {
        const rpg::TileCoordinates candidate{origin.x + offset.x, origin.y + offset.y};

        if (world.isInBounds(candidate) && world.isTraversable(candidate) == wantTraversable)
        {
            return candidate;
        }
    }

    return std::nullopt;
}

[[nodiscard]] rpg::MovementIntent movementIntentForTiles(
    const rpg::TileCoordinates& from,
    const rpg::TileCoordinates& to) noexcept
{
    return {
        static_cast<float>(to.x - from.x),
        static_cast<float>(to.y - from.y)};
}

[[nodiscard]] bool hasAdjacentTileType(
    const rpg::World& world,
    const rpg::TileCoordinates& coordinates,
    const rpg::TileType tileType)
{
    constexpr rpg::TileCoordinates kOffsets[] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}};

    for (const rpg::TileCoordinates& offset : kOffsets)
    {
        const rpg::TileCoordinates candidate{coordinates.x + offset.x, coordinates.y + offset.y};

        if (world.isInBounds(candidate) && world.getTileType(candidate) == tileType)
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool verifyDeterministicWorld()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    rpg::World firstWorld(config);
    rpg::World secondWorld(config);

    if (firstWorld.getWidthInTiles() != secondWorld.getWidthInTiles()
        || firstWorld.getHeightInTiles() != secondWorld.getHeightInTiles()
        || !areClose(firstWorld.getTileSize(), secondWorld.getTileSize())
        || firstWorld.getSpawnTile().x != secondWorld.getSpawnTile().x
        || firstWorld.getSpawnTile().y != secondWorld.getSpawnTile().y)
    {
        return false;
    }

    for (int y = 0; y < firstWorld.getHeightInTiles(); ++y)
    {
        for (int x = 0; x < firstWorld.getWidthInTiles(); ++x)
        {
            const rpg::TileCoordinates coordinates{x, y};

            if (firstWorld.getTileType(coordinates) != secondWorld.getTileType(coordinates))
            {
                return false;
            }
        }
    }

    return true;
}

[[nodiscard]] bool verifyConfiguredWorldDimensions()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 24, .heightInTiles = 18, .tileSize = 16.0F};
    rpg::World world(config);
    const rpg::WorldSize worldSize = world.getWorldSize();
    const rpg::TileCoordinates cornerTile{config.widthInTiles - 1, config.heightInTiles - 1};
    const rpg::WorldPosition cornerCenter = world.getTileCenter(cornerTile);
    const rpg::TileCoordinates recoveredTile = world.getTileCoordinates(cornerCenter);

    return world.getWidthInTiles() == config.widthInTiles
        && world.getHeightInTiles() == config.heightInTiles
        && areClose(world.getTileSize(), config.tileSize)
        && areClose(worldSize.width, static_cast<float>(config.widthInTiles) * config.tileSize)
        && areClose(worldSize.height, static_cast<float>(config.heightInTiles) * config.tileSize)
        && recoveredTile.x == cornerTile.x
        && recoveredTile.y == cornerTile.y;
}

[[nodiscard]] bool verifySpawnValidity()
{
    const auto hasValidSpawn = [](const rpg::World& world)
    {
        const rpg::TileCoordinates spawnTile = world.getSpawnTile();
        const rpg::WorldPosition spawnPosition = world.getSpawnPosition();

        return world.isInBounds(spawnTile)
            && world.isTraversable(spawnTile)
            && world.getTileCoordinates(spawnPosition).x == spawnTile.x
            && world.getTileCoordinates(spawnPosition).y == spawnTile.y;
    };

    rpg::World defaultWorld;
    const rpg::WorldConfig configuredWorldConfig{
        .seed = 0x13572468U,
        .widthInTiles = 36,
        .heightInTiles = 20,
        .tileSize = 20.0F};
    rpg::World configuredWorld(configuredWorldConfig);

    return hasValidSpawn(defaultWorld) && hasValidSpawn(configuredWorld);
}

[[nodiscard]] bool verifyDefaultWorldConfiguration()
{
    rpg::World firstWorld;
    rpg::World secondWorld;

    return firstWorld.getWidthInTiles() == 64
        && firstWorld.getHeightInTiles() == 64
        && areClose(firstWorld.getTileSize(), 32.0F)
        && firstWorld.getSpawnTile().x == secondWorld.getSpawnTile().x
        && firstWorld.getSpawnTile().y == secondWorld.getSpawnTile().y;
}

[[nodiscard]] bool verifyPlayerMovement()
{
    rpg::World world;
    rpg::Player player;

    const rpg::TileCoordinates spawnTile = world.getSpawnTile();
    const std::optional<rpg::TileCoordinates> traversableNeighbor = findAdjacentTile(world, spawnTile, true);

    if (!traversableNeighbor.has_value())
    {
        return false;
    }

    player.spawn(world.getTileCenter(spawnTile));
    player.setMovementIntent(movementIntentForTiles(spawnTile, *traversableNeighbor));
    player.update(world.getTileSize() / player.getMovementSpeed(), world);

    const rpg::TileCoordinates movedTile = world.getTileCoordinates(player.getPosition());

    if (movedTile.x != traversableNeighbor->x || movedTile.y != traversableNeighbor->y)
    {
        return false;
    }

    for (int y = 1; y < world.getHeightInTiles() - 1; ++y)
    {
        for (int x = 1; x < world.getWidthInTiles() - 1; ++x)
        {
            const rpg::TileCoordinates coordinates{x, y};

            if (!world.isTraversable(coordinates))
            {
                continue;
            }

            const std::optional<rpg::TileCoordinates> blockedNeighbor = findAdjacentTile(world, coordinates, false);

            if (!blockedNeighbor.has_value())
            {
                continue;
            }

            player.spawn(world.getTileCenter(coordinates));
            player.setMovementIntent(movementIntentForTiles(coordinates, *blockedNeighbor));
            player.update(world.getTileSize() / player.getMovementSpeed(), world);

            const rpg::TileCoordinates blockedResult = world.getTileCoordinates(player.getPosition());

            return blockedResult.x == coordinates.x && blockedResult.y == coordinates.y;
        }
    }

    return false;
}

[[nodiscard]] bool verifyCameraClamping()
{
    rpg::World world;
    rpg::Camera camera;

    const rpg::WorldSize worldSize = world.getWorldSize();

    camera.update({0.0F, 0.0F}, world, 1280.0F, 720.0F);
    const rpg::ViewFrame topLeftFrame = camera.getFrame();

    if (!areClose(topLeftFrame.center.x, topLeftFrame.size.width * 0.5F)
        || !areClose(topLeftFrame.center.y, topLeftFrame.size.height * 0.5F))
    {
        return false;
    }

    camera.update({worldSize.width, worldSize.height}, world, 1280.0F, 720.0F);
    const rpg::ViewFrame bottomRightFrame = camera.getFrame();

    if (!areClose(bottomRightFrame.center.x, worldSize.width - (bottomRightFrame.size.width * 0.5F))
        || !areClose(bottomRightFrame.center.y, worldSize.height - (bottomRightFrame.size.height * 0.5F)))
    {
        return false;
    }

    return true;
}

[[nodiscard]] bool verifySignalDrivenBiomeDistribution()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    rpg::World world(config);

    bool foundShorelineSand = false;
    bool foundInlandGrass = false;
    bool foundInlandForest = false;

    for (int y = 1; y < world.getHeightInTiles() - 1; ++y)
    {
        for (int x = 1; x < world.getWidthInTiles() - 1; ++x)
        {
            const rpg::TileCoordinates coordinates{x, y};
            const rpg::TileType tileType = world.getTileType(coordinates);
            const bool adjacentToWater = hasAdjacentTileType(world, coordinates, rpg::TileType::Water);
            const bool isInland = x > 2
                && x < world.getWidthInTiles() - 3
                && y > 2
                && y < world.getHeightInTiles() - 3
                && !adjacentToWater;

            if (tileType == rpg::TileType::Sand && adjacentToWater)
            {
                foundShorelineSand = true;
            }

            if (!isInland)
            {
                continue;
            }

            if (tileType == rpg::TileType::Grass)
            {
                foundInlandGrass = true;
            }
            else if (tileType == rpg::TileType::Forest)
            {
                foundInlandForest = true;
            }
        }
    }

    return foundShorelineSand && foundInlandGrass && foundInlandForest;
}

} // namespace

int main()
{
    if (!verifyDeterministicWorld())
    {
        return 1;
    }

    if (!verifySpawnValidity())
    {
        return 1;
    }

    if (!verifyConfiguredWorldDimensions())
    {
        return 1;
    }

    if (!verifyDefaultWorldConfiguration())
    {
        return 1;
    }

    if (!verifyPlayerMovement())
    {
        return 1;
    }

    if (!verifyCameraClamping())
    {
        return 1;
    }

    if (!verifySignalDrivenBiomeDistribution())
    {
        return 1;
    }

    return 0;
}
