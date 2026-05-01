/**
 * @file OverworldRuntimeTests.cpp
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

#include <main/OverworldRuntime.hpp>

#include <array>
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
    constexpr std::array<rpg::TileCoordinates, 4> kOffsets = {{
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    }};

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

[[nodiscard]] rpg::MovementIntent movementIntentForTiles(
    const rpg::TileCoordinates& from,
    const rpg::TileCoordinates& to) noexcept
{
    return {
        static_cast<float>(to.x - from.x),
        static_cast<float>(to.y - from.y)};
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

[[nodiscard]] bool verifySessionInitialization()
{
    rpg::OverworldRuntime runtime;
    rpg::World world;
    runtime.initialize({1280.0F, 720.0F});
    const rpg::OverworldFrameState& frameState = runtime.getFrameState();
    const rpg::WorldPosition spawnPosition = world.getSpawnPosition();

    return areClose(frameState.tileSize, world.getTileSize())
        && areClose(frameState.frame.center.x, spawnPosition.x)
        && areClose(frameState.frame.center.y, spawnPosition.y)
        && areClose(frameState.frame.size.width, 1280.0F)
        && areClose(frameState.frame.size.height, 720.0F)
        && areClose(frameState.playerMarker.position.x, spawnPosition.x)
        && areClose(frameState.playerMarker.position.y, spawnPosition.y)
        && containsVisibleTile(frameState.visibleTiles, world.getSpawnTile());
}

[[nodiscard]] bool verifyGameplayProgression()
{
    rpg::OverworldRuntime runtime;
    rpg::World world;
    rpg::Player player;
    const rpg::TileCoordinates spawnTile = world.getSpawnTile();
    const std::optional<rpg::TileCoordinates> traversableNeighbor = findAdjacentTile(world, spawnTile, true);

    if (!traversableNeighbor.has_value())
    {
        return false;
    }

    runtime.initialize({320.0F, 224.0F});
    runtime.update(
        world.getTileSize() / player.getMovementSpeed(),
        {
            movementIntentForTiles(spawnTile, *traversableNeighbor),
            {320.0F, 224.0F}});

    const rpg::OverworldFrameState& frameState = runtime.getFrameState();
    const rpg::TileCoordinates movedTile = world.getTileCoordinates(frameState.playerMarker.position);

    return movedTile.x == traversableNeighbor->x
        && movedTile.y == traversableNeighbor->y
        && areClose(frameState.frame.center.x, frameState.playerMarker.position.x)
        && areClose(frameState.frame.center.y, frameState.playerMarker.position.y);
}

[[nodiscard]] bool verifyRenderFacingFrameState()
{
    rpg::OverworldRuntime runtime;
    runtime.initialize({1280.0F, 720.0F});
    runtime.update(
        0.0F,
        {
            {0.0F, 0.0F},
            {320.0F, 224.0F}});

    const rpg::OverworldFrameState& frameState = runtime.getFrameState();

    return areClose(frameState.frame.size.width, 320.0F)
        && areClose(frameState.frame.size.height, 224.0F)
        && areClose(frameState.playerMarker.size.width, frameState.tileSize * 0.5F)
        && areClose(frameState.playerMarker.size.height, frameState.tileSize * 0.5F)
        && !frameState.visibleTiles.empty();
}

} // namespace

int main()
{
    if (!verifySessionInitialization())
    {
        return 1;
    }

    if (!verifyGameplayProgression())
    {
        return 1;
    }

    if (!verifyRenderFacingFrameState())
    {
        return 1;
    }

    return 0;
}
