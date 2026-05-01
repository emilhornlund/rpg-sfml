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

#include <array>
#include <cmath>
#include <optional>
#include <vector>

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

        if (world.isTraversable(candidate) == wantTraversable)
        {
            return candidate;
        }
    }

    return std::nullopt;
}

[[nodiscard]] std::optional<rpg::TileCoordinates> findTraversableTileNear(
    const rpg::World& world,
    const rpg::TileCoordinates& anchor,
    const int searchRadius)
{
    for (int radius = 0; radius <= searchRadius; ++radius)
    {
        for (int y = anchor.y - radius; y <= anchor.y + radius; ++y)
        {
            for (int x = anchor.x - radius; x <= anchor.x + radius; ++x)
            {
                if (radius > 0
                    && x != anchor.x - radius
                    && x != anchor.x + radius
                    && y != anchor.y - radius
                    && y != anchor.y + radius)
                {
                    continue;
                }

                const rpg::TileCoordinates coordinates{x, y};

                if (world.isTraversable(coordinates))
                {
                    return coordinates;
                }
            }
        }
    }

    return std::nullopt;
}

struct TraversableRun
{
    rpg::TileCoordinates start;
    rpg::MovementIntent movementIntent;
};

[[nodiscard]] std::optional<TraversableRun> findTraversableRun(
    const rpg::World& world,
    const rpg::TileCoordinates& anchor,
    const int searchRadius,
    const int runLength)
{
    constexpr std::array<rpg::TileCoordinates, 4> kDirections = {{
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    }};

    for (int radius = 0; radius <= searchRadius; ++radius)
    {
        for (int y = anchor.y - radius; y <= anchor.y + radius; ++y)
        {
            for (int x = anchor.x - radius; x <= anchor.x + radius; ++x)
            {
                if (radius > 0
                    && x != anchor.x - radius
                    && x != anchor.x + radius
                    && y != anchor.y - radius
                    && y != anchor.y + radius)
                {
                    continue;
                }

                const rpg::TileCoordinates start{x, y};

                if (!world.isTraversable(start))
                {
                    continue;
                }

                for (const rpg::TileCoordinates& direction : kDirections)
                {
                    bool runIsTraversable = true;

                    for (int step = 1; step <= runLength; ++step)
                    {
                        const rpg::TileCoordinates candidate{
                            start.x + direction.x * step,
                            start.y + direction.y * step};

                        if (!world.isTraversable(candidate))
                        {
                            runIsTraversable = false;
                            break;
                        }
                    }

                    if (runIsTraversable)
                    {
                        return TraversableRun{
                            start,
                            {static_cast<float>(direction.x), static_cast<float>(direction.y)}};
                    }
                }
            }
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

        if (world.getTileType(candidate) == tileType)
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
    const std::array<rpg::TileCoordinates, 5> sampleCoordinates = {{
        {-24, -12},
        {-1, 0},
        {0, 0},
        {16, 8},
        {72, 40},
    }};

    if (!areClose(firstWorld.getTileSize(), secondWorld.getTileSize())
        || firstWorld.getSpawnTile().x != secondWorld.getSpawnTile().x
        || firstWorld.getSpawnTile().y != secondWorld.getSpawnTile().y)
    {
        return false;
    }

    for (const rpg::TileCoordinates& coordinates : sampleCoordinates)
    {
        if (firstWorld.getTileType(coordinates) != secondWorld.getTileType(coordinates))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool verifyConfiguredWorldCoordinates()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 24, .heightInTiles = 18, .tileSize = 16.0F};
    rpg::World world(config);
    const rpg::TileCoordinates tile{24, -18};
    const rpg::WorldPosition tileCenter = world.getTileCenter(tile);
    const rpg::TileCoordinates recoveredTile = world.getTileCoordinates(tileCenter);

    return areClose(world.getTileSize(), config.tileSize)
        && areClose(tileCenter.x, (static_cast<float>(tile.x) + 0.5F) * config.tileSize)
        && areClose(tileCenter.y, (static_cast<float>(tile.y) + 0.5F) * config.tileSize)
        && recoveredTile.x == tile.x
        && recoveredTile.y == tile.y;
}

[[nodiscard]] bool verifySpawnValidity()
{
    const auto hasValidSpawn = [](const rpg::World& world)
    {
        const rpg::TileCoordinates spawnTile = world.getSpawnTile();
        const rpg::WorldPosition spawnPosition = world.getSpawnPosition();

        return world.isTraversable(spawnTile)
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

    return areClose(firstWorld.getTileSize(), 16.0F)
        && firstWorld.getSpawnTile().x == secondWorld.getSpawnTile().x
        && firstWorld.getSpawnTile().y == secondWorld.getSpawnTile().y;
}

[[nodiscard]] bool verifyChunkMetadataQueries()
{
    const rpg::WorldConfig config{.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F};
    rpg::World firstWorld(config);
    rpg::World secondWorld(config);
    const rpg::TileCoordinates sampleTile{36, -19};
    const rpg::ChunkCoordinates sampleChunk = firstWorld.getChunkCoordinates(sampleTile);
    const rpg::ChunkMetadata firstMetadata = firstWorld.getChunkMetadata(sampleChunk);
    const rpg::ChunkMetadata repeatedMetadata = firstWorld.getChunkMetadata(sampleTile);
    const rpg::ChunkMetadata secondMetadata = secondWorld.getChunkMetadata(sampleChunk);

    if (firstMetadata.chunkCoordinates.x != sampleChunk.x
        || firstMetadata.chunkCoordinates.y != sampleChunk.y
        || repeatedMetadata.chunkCoordinates.x != sampleChunk.x
        || repeatedMetadata.chunkCoordinates.y != sampleChunk.y
        || secondMetadata.chunkCoordinates.x != sampleChunk.x
        || secondMetadata.chunkCoordinates.y != sampleChunk.y)
    {
        return false;
    }

    if (firstMetadata.biomeSummary.dominantTileType != repeatedMetadata.biomeSummary.dominantTileType
        || firstMetadata.biomeSummary.dominantTileType != secondMetadata.biomeSummary.dominantTileType
        || firstMetadata.traversabilitySummary.traversableTileCount != repeatedMetadata.traversabilitySummary.traversableTileCount
        || firstMetadata.traversabilitySummary.traversableTileCount != secondMetadata.traversabilitySummary.traversableTileCount
        || firstMetadata.candidates.size() != repeatedMetadata.candidates.size()
        || firstMetadata.candidates.size() != secondMetadata.candidates.size())
    {
        return false;
    }

    return firstMetadata.biomeSummary.waterTileCount == repeatedMetadata.biomeSummary.waterTileCount
        && firstMetadata.biomeSummary.waterTileCount == secondMetadata.biomeSummary.waterTileCount
        && firstMetadata.traversabilitySummary.blockedTileCount == repeatedMetadata.traversabilitySummary.blockedTileCount
        && firstMetadata.traversabilitySummary.blockedTileCount == secondMetadata.traversabilitySummary.blockedTileCount;
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

    for (int radius = 0; radius <= 32; ++radius)
    {
        for (int y = spawnTile.y - radius; y <= spawnTile.y + radius; ++y)
        {
            for (int x = spawnTile.x - radius; x <= spawnTile.x + radius; ++x)
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
    }

    return false;
}

[[nodiscard]] bool verifyPlayerMovementBeyondInitialArea()
{
    rpg::World world;
    rpg::Player player;
    const std::optional<rpg::TileCoordinates> originNearFarTile = findTraversableTileNear(world, {96, 64}, 32);

    if (!originNearFarTile.has_value())
    {
        return false;
    }

    const std::optional<rpg::TileCoordinates> traversableNeighbor = findAdjacentTile(world, *originNearFarTile, true);

    if (!traversableNeighbor.has_value())
    {
        return false;
    }

    player.spawn(world.getTileCenter(*originNearFarTile));
    player.setMovementIntent(movementIntentForTiles(*originNearFarTile, *traversableNeighbor));
    player.update(world.getTileSize() / player.getMovementSpeed(), world);

    const rpg::TileCoordinates movedTile = world.getTileCoordinates(player.getPosition());
    return movedTile.x == traversableNeighbor->x && movedTile.y == traversableNeighbor->y;
}

[[nodiscard]] bool verifyPlayerWalkAnimationSequence()
{
    constexpr float kAnimationStepSeconds = 0.12F;

    rpg::World world;
    rpg::Player player;
    const std::optional<TraversableRun> traversableRun = findTraversableRun(world, world.getSpawnTile(), 32, 8);

    if (!traversableRun.has_value())
    {
        return false;
    }

    player.spawn(world.getTileCenter(traversableRun->start));
    player.setMovementIntent(traversableRun->movementIntent);

    if (player.getWalkFrameIndex() != 1 || player.isMoving())
    {
        return false;
    }

    constexpr std::array<int, 4> kExpectedMovingFrames = {2, 1, 0, 1};

    for (const int expectedFrame : kExpectedMovingFrames)
    {
        player.update(kAnimationStepSeconds, world);

        if (!player.isMoving() || player.getWalkFrameIndex() != expectedFrame)
        {
            return false;
        }
    }

    player.setMovementIntent({0.0F, 0.0F});
    player.update(0.0F, world);

    return !player.isMoving() && player.getWalkFrameIndex() == 1;
}

[[nodiscard]] bool verifyCameraFollowsFocus()
{
    rpg::Camera camera;
    const rpg::WorldPosition firstFocus{0.0F, 0.0F};
    const rpg::WorldPosition secondFocus{384.0F, 288.0F};

    camera.update(firstFocus, 1280.0F, 720.0F);
    const rpg::ViewFrame firstFrame = camera.getFrame();
    camera.update(secondFocus, 320.0F, 224.0F);
    const rpg::ViewFrame secondFrame = camera.getFrame();

    return areClose(firstFrame.center.x, firstFocus.x)
        && areClose(firstFrame.center.y, firstFocus.y)
        && areClose(firstFrame.size.width, 1280.0F)
        && areClose(firstFrame.size.height, 720.0F)
        && areClose(secondFrame.center.x, secondFocus.x)
        && areClose(secondFrame.center.y, secondFocus.y)
        && areClose(secondFrame.size.width, 320.0F)
        && areClose(secondFrame.size.height, 224.0F);
}

[[nodiscard]] bool verifySignalDrivenBiomeDistribution()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    rpg::World world(config);

    bool foundShorelineSand = false;
    bool foundInlandGrass = false;
    bool foundInlandForest = false;

    for (int y = -32; y <= 32; ++y)
    {
        for (int x = -32; x <= 32; ++x)
        {
            const rpg::TileCoordinates coordinates{x, y};
            const rpg::TileType tileType = world.getTileType(coordinates);
            const bool adjacentToWater = hasAdjacentTileType(world, coordinates, rpg::TileType::Water);
            const bool isInland = std::abs(x) > 4
                && std::abs(y) > 4
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

[[nodiscard]] bool containsVisibleTile(
    const std::vector<rpg::VisibleWorldTile>& visibleTiles,
    const rpg::TileCoordinates& coordinates) noexcept
{
    for (const rpg::VisibleWorldTile& visibleTile : visibleTiles)
    {
        if (visibleTile.coordinates.x == coordinates.x && visibleTile.coordinates.y == coordinates.y)
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool verifyVisibleTerrainTraversal()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 64, .heightInTiles = 64, .tileSize = 32.0F};
    rpg::World world(config);
    rpg::Camera camera;

    camera.update(world.getSpawnPosition(), 320.0F, 224.0F);
    const std::vector<rpg::VisibleWorldTile> visibleTiles = world.getVisibleTiles(camera.getFrame());

    if (visibleTiles.empty()
        || !containsVisibleTile(visibleTiles, world.getSpawnTile()))
    {
        return false;
    }

    for (const rpg::VisibleWorldTile& visibleTile : visibleTiles)
    {
        if (visibleTile.tileType != world.getTileType(visibleTile.coordinates))
        {
            return false;
        }

        const rpg::TileCoordinates recoveredCoordinates = world.getTileCoordinates(visibleTile.center);

        if (recoveredCoordinates.x != visibleTile.coordinates.x || recoveredCoordinates.y != visibleTile.coordinates.y)
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool verifyVisibleTerrainOverscan()
{
    const rpg::WorldConfig config{.seed = 0x13572468U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 20.0F};
    rpg::World world(config);
    const rpg::ViewFrame frame{
        world.getTileCenter({10, 10}),
        {world.getTileSize(), world.getTileSize()}};
    const std::vector<rpg::VisibleWorldTile> visibleTiles = world.getVisibleTiles(frame);

    return visibleTiles.size() == 9
        && containsVisibleTile(visibleTiles, {9, 9})
        && containsVisibleTile(visibleTiles, {10, 10})
        && containsVisibleTile(visibleTiles, {11, 11})
        && !containsVisibleTile(visibleTiles, {8, 10})
        && !containsVisibleTile(visibleTiles, {12, 10});
}

[[nodiscard]] bool verifyVisibleTerrainSupportsFarFrames()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 24, .heightInTiles = 18, .tileSize = 16.0F};
    rpg::World world(config);
    rpg::Camera camera;

    camera.update(world.getTileCenter({-48, 32}), 128.0F, 96.0F);
    const std::vector<rpg::VisibleWorldTile> negativeVisibleTiles = world.getVisibleTiles(camera.getFrame());

    if (negativeVisibleTiles.empty()
        || !containsVisibleTile(negativeVisibleTiles, {-48, 32}))
    {
        return false;
    }

    camera.update(world.getTileCenter({96, -72}), 128.0F, 96.0F);
    const std::vector<rpg::VisibleWorldTile> positiveVisibleTiles = world.getVisibleTiles(camera.getFrame());

    return !positiveVisibleTiles.empty()
        && containsVisibleTile(positiveVisibleTiles, {96, -72});
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

    if (!verifyConfiguredWorldCoordinates())
    {
        return 1;
    }

    if (!verifyDefaultWorldConfiguration())
    {
        return 1;
    }

    if (!verifyChunkMetadataQueries())
    {
        return 1;
    }

    if (!verifyPlayerMovement())
    {
        return 1;
    }

    if (!verifyPlayerMovementBeyondInitialArea())
    {
        return 1;
    }

    if (!verifyPlayerWalkAnimationSequence())
    {
        return 1;
    }

    if (!verifyCameraFollowsFocus())
    {
        return 1;
    }

    if (!verifySignalDrivenBiomeDistribution())
    {
        return 1;
    }

    if (!verifyVisibleTerrainTraversal())
    {
        return 1;
    }

    if (!verifyVisibleTerrainOverscan())
    {
        return 1;
    }

    if (!verifyVisibleTerrainSupportsFarFrames())
    {
        return 1;
    }

    return 0;
}
