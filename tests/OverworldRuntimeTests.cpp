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
    const std::vector<rpg::OverworldRenderTile>& visibleTiles,
    const rpg::TileCoordinates& coordinates) noexcept
{
    for (const rpg::OverworldRenderTile& visibleTile : visibleTiles)
    {
        if (visibleTile.coordinates.x == coordinates.x && visibleTile.coordinates.y == coordinates.y)
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] const rpg::OverworldRenderMarker* findPlayerMarker(
    const std::vector<rpg::OverworldRenderMarker>& markers) noexcept
{
    for (const rpg::OverworldRenderMarker& marker : markers)
    {
        if (marker.appearance == rpg::OverworldRenderMarkerAppearance::Player)
        {
            return &marker;
        }
    }

    return nullptr;
}

[[nodiscard]] bool verifySessionInitialization()
{
    rpg::OverworldRuntime runtime;
    rpg::World world;
    runtime.initialize({1280.0F, 720.0F});
    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const rpg::WorldPosition spawnPosition = world.getSpawnPosition();
    const rpg::OverworldRenderMarker* playerMarker = findPlayerMarker(renderSnapshot.markers);

    return playerMarker != nullptr
        && areClose(renderSnapshot.cameraFrame.center.x, spawnPosition.x)
        && areClose(renderSnapshot.cameraFrame.center.y, spawnPosition.y)
        && areClose(renderSnapshot.cameraFrame.size.width, 1280.0F)
        && areClose(renderSnapshot.cameraFrame.size.height, 720.0F)
        && areClose(playerMarker->position.x, spawnPosition.x)
        && areClose(playerMarker->position.y, spawnPosition.y)
        && containsVisibleTile(renderSnapshot.visibleTiles, world.getSpawnTile());
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

    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const rpg::OverworldRenderMarker* playerMarker = findPlayerMarker(renderSnapshot.markers);

    if (playerMarker == nullptr)
    {
        return false;
    }

    const rpg::TileCoordinates movedTile = world.getTileCoordinates(playerMarker->position);

    return movedTile.x == traversableNeighbor->x
        && movedTile.y == traversableNeighbor->y
        && areClose(renderSnapshot.cameraFrame.center.x, playerMarker->position.x)
        && areClose(renderSnapshot.cameraFrame.center.y, playerMarker->position.y);
}

[[nodiscard]] bool verifyRenderSnapshotContents()
{
    rpg::OverworldRuntime runtime;
    runtime.initialize({1280.0F, 720.0F});
    runtime.update(
        0.0F,
        {
            {0.0F, 0.0F},
            {320.0F, 224.0F}});

    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const rpg::OverworldRenderMarker* playerMarker = findPlayerMarker(renderSnapshot.markers);

    if (playerMarker == nullptr || renderSnapshot.visibleTiles.empty())
    {
        return false;
    }

    const rpg::OverworldRenderTile& firstVisibleTile = renderSnapshot.visibleTiles.front();

    return areClose(renderSnapshot.cameraFrame.size.width, 320.0F)
        && areClose(renderSnapshot.cameraFrame.size.height, 224.0F)
        && areClose(firstVisibleTile.size.width, 32.0F)
        && areClose(firstVisibleTile.size.height, 32.0F)
        && areClose(firstVisibleTile.origin.x, 16.0F)
        && areClose(firstVisibleTile.origin.y, 16.0F)
        && areClose(playerMarker->size.width, 16.0F)
        && areClose(playerMarker->size.height, 16.0F)
        && renderSnapshot.markers.size() == 1;
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

    if (!verifyRenderSnapshotContents())
    {
        return 1;
    }

    return 0;
}
