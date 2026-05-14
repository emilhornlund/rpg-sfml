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

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <optional>
#include <queue>
#include <utility>

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

[[nodiscard]] std::pair<int, int> makeTileKey(const rpg::TileCoordinates& coordinates) noexcept
{
    return {coordinates.x, coordinates.y};
}

[[nodiscard]] rpg::TileCoordinates makeTileCoordinates(const std::pair<int, int>& key) noexcept
{
    return {key.first, key.second};
}

[[nodiscard]] std::optional<std::vector<rpg::TileCoordinates>> findTraversablePathWithMinimumLength(
    const rpg::World& world,
    const rpg::TileCoordinates& start,
    const int minimumLength,
    const int maximumLength)
{
    if (!world.isTraversable(start) || minimumLength < 0 || maximumLength < minimumLength)
    {
        return std::nullopt;
    }

    constexpr std::array<rpg::TileCoordinates, 4> kOffsets = {{
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    }};

    const std::pair<int, int> startKey = makeTileKey(start);
    std::queue<std::pair<int, int>> frontier;
    std::map<std::pair<int, int>, std::pair<int, int>> parents;
    std::map<std::pair<int, int>, int> stepsFromStart;
    frontier.push(startKey);
    parents.emplace(startKey, startKey);
    stepsFromStart.emplace(startKey, 0);

    while (!frontier.empty())
    {
        const std::pair<int, int> currentKey = frontier.front();
        frontier.pop();
        const int stepCount = stepsFromStart.at(currentKey);

        if (stepCount >= minimumLength)
        {
            std::vector<rpg::TileCoordinates> path;
            std::pair<int, int> pathKey = currentKey;

            while (pathKey != startKey)
            {
                path.push_back(makeTileCoordinates(pathKey));
                pathKey = parents.at(pathKey);
            }

            std::reverse(path.begin(), path.end());
            return path;
        }

        if (stepCount >= maximumLength)
        {
            continue;
        }

        const rpg::TileCoordinates current = makeTileCoordinates(currentKey);

        for (const rpg::TileCoordinates& offset : kOffsets)
        {
            const rpg::TileCoordinates candidate{current.x + offset.x, current.y + offset.y};

            if (std::max(std::abs(candidate.x - start.x), std::abs(candidate.y - start.y)) > maximumLength
                || !world.isTraversable(candidate))
            {
                continue;
            }

            const std::pair<int, int> candidateKey = makeTileKey(candidate);

            if (stepsFromStart.contains(candidateKey))
            {
                continue;
            }

            parents.emplace(candidateKey, currentKey);
            stepsFromStart.emplace(candidateKey, stepCount + 1);
            frontier.push(candidateKey);
        }
    }

    return std::nullopt;
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

[[nodiscard]] const rpg::OverworldRenderContent* findGeneratedContent(
    const std::vector<rpg::OverworldRenderContent>& generatedContent,
    const std::uint64_t id) noexcept
{
    for (const rpg::OverworldRenderContent& renderContent : generatedContent)
    {
        if (renderContent.id == id)
        {
            return &renderContent;
        }
    }

    return nullptr;
}

[[nodiscard]] rpg::PlayerFacingDirection getFacingDirectionForNeighbor(
    const rpg::TileCoordinates& origin,
    const rpg::TileCoordinates& neighbor) noexcept
{
    if (neighbor.x < origin.x)
    {
        return rpg::PlayerFacingDirection::Left;
    }

    if (neighbor.x > origin.x)
    {
        return rpg::PlayerFacingDirection::Right;
    }

    if (neighbor.y < origin.y)
    {
        return rpg::PlayerFacingDirection::Up;
    }

    return rpg::PlayerFacingDirection::Down;
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
        && areClose(renderSnapshot.cameraFrame.size.width, 1280.0F / 3.0F)
        && areClose(renderSnapshot.cameraFrame.size.height, 720.0F / 3.0F)
        && areClose(playerMarker->position.x, spawnPosition.x)
        && areClose(playerMarker->position.y, spawnPosition.y)
        && areClose(playerMarker->sortKeyY, spawnPosition.y)
        && playerMarker->facingDirection == rpg::PlayerFacingDirection::Down
        && playerMarker->animationFrameIndex == 1
        && containsVisibleTile(renderSnapshot.visibleTiles, world.getSpawnTile());
}

[[nodiscard]] bool verifyRepeatedInitializationRefreshesPublishedSnapshot()
{
    rpg::OverworldRuntime runtime;
    rpg::World world;
    const rpg::WorldPosition spawnPosition = world.getSpawnPosition();

    runtime.initialize({1280.0F, 720.0F});
    const rpg::ViewFrame initialFrame = runtime.getRenderSnapshot().cameraFrame;

    runtime.initialize({1600.0F, 900.0F});
    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const rpg::OverworldRenderMarker* playerMarker = findPlayerMarker(renderSnapshot.markers);

    return playerMarker != nullptr
        && areClose(initialFrame.size.width, 1280.0F / 3.0F)
        && areClose(initialFrame.size.height, 720.0F / 3.0F)
        && areClose(renderSnapshot.cameraFrame.center.x, spawnPosition.x)
        && areClose(renderSnapshot.cameraFrame.center.y, spawnPosition.y)
        && areClose(renderSnapshot.cameraFrame.size.width, 1600.0F / 3.0F)
        && areClose(renderSnapshot.cameraFrame.size.height, 900.0F / 3.0F)
        && areClose(playerMarker->position.x, spawnPosition.x)
        && areClose(playerMarker->position.y, spawnPosition.y);
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
        && areClose(renderSnapshot.cameraFrame.center.y, playerMarker->position.y)
        && playerMarker->facingDirection == getFacingDirectionForNeighbor(spawnTile, *traversableNeighbor)
        && playerMarker->animationFrameIndex >= 0
        && playerMarker->animationFrameIndex <= 2;
}

[[nodiscard]] bool verifyRuntimeCompletesReleasedStep()
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

    const float halfStepSeconds = world.getTileSize() / player.getMovementSpeed() * 0.5F;
    runtime.initialize({320.0F, 224.0F});
    runtime.update(
        halfStepSeconds,
        {
            movementIntentForTiles(spawnTile, *traversableNeighbor),
            {320.0F, 224.0F}});

    const rpg::OverworldRenderMarker* midStepMarker = findPlayerMarker(runtime.getRenderSnapshot().markers);
    rpg::WorldPosition midStepPosition{0.0F, 0.0F};

    if (midStepMarker == nullptr)
    {
        return false;
    }

    midStepPosition = midStepMarker->position;

    runtime.update(
        halfStepSeconds,
        {
            {0.0F, 0.0F},
            {320.0F, 224.0F}});

    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const rpg::OverworldRenderMarker* playerMarker = findPlayerMarker(renderSnapshot.markers);

    if (playerMarker == nullptr)
    {
        return false;
    }

    const rpg::WorldPosition destinationCenter = world.getTileCenter(*traversableNeighbor);

    return !areClose(midStepPosition.x, destinationCenter.x)
        || !areClose(midStepPosition.y, destinationCenter.y)
        ? areClose(playerMarker->position.x, destinationCenter.x)
            && areClose(playerMarker->position.y, destinationCenter.y)
            && areClose(renderSnapshot.cameraFrame.center.x, destinationCenter.x)
            && areClose(renderSnapshot.cameraFrame.center.y, destinationCenter.y)
        : false;
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

    return areClose(renderSnapshot.cameraFrame.size.width, 320.0F / 3.0F)
        && areClose(renderSnapshot.cameraFrame.size.height, 224.0F / 3.0F)
        && areClose(firstVisibleTile.size.width, 16.0F)
        && areClose(firstVisibleTile.size.height, 16.0F)
        && areClose(firstVisibleTile.origin.x, 8.0F)
        && areClose(firstVisibleTile.origin.y, 8.0F)
        && areClose(playerMarker->size.width, 48.0F)
        && areClose(playerMarker->size.height, 48.0F)
        && areClose(playerMarker->origin.x, 24.0F)
        && areClose(playerMarker->origin.y, 32.0F)
        && areClose(playerMarker->sortKeyY, playerMarker->position.y)
        && playerMarker->facingDirection == rpg::PlayerFacingDirection::Down
        && playerMarker->animationFrameIndex == 1
        && renderSnapshot.markers.size() == 1;
}

[[nodiscard]] bool verifyDebugZoomAdjustsPublishedCameraFrame()
{
    rpg::OverworldRuntime runtime;

    runtime.update(
        0.0F,
        {
            {0.0F, 0.0F},
            {1200.0F, 900.0F},
            {true, 300, false}});
    const rpg::ViewFrame zoomedInFrame = runtime.getRenderSnapshot().cameraFrame;

    runtime.update(
        0.0F,
        {
            {0.0F, 0.0F},
            {1200.0F, 900.0F},
            {true, 150, false}});
    const rpg::ViewFrame zoomedOutFrame = runtime.getRenderSnapshot().cameraFrame;

    return areClose(zoomedInFrame.size.width, 400.0F)
        && areClose(zoomedInFrame.size.height, 300.0F)
        && areClose(zoomedOutFrame.size.width, 800.0F)
        && areClose(zoomedOutFrame.size.height, 600.0F);
}

[[nodiscard]] bool verifyInitializedRuntimePublishesUpdatedViewportAndZoom()
{
    rpg::OverworldRuntime runtime;
    rpg::World world;
    runtime.initialize({1280.0F, 720.0F});

    runtime.update(
        0.0F,
        {
            {0.0F, 0.0F},
            {1600.0F, 900.0F},
            {true, 150, false}});

    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const rpg::OverworldDebugSnapshot& debugSnapshot = runtime.getDebugSnapshot();
    const rpg::OverworldRenderMarker* playerMarker = findPlayerMarker(renderSnapshot.markers);
    const rpg::WorldPosition spawnPosition = world.getSpawnPosition();

    return playerMarker != nullptr
        && areClose(renderSnapshot.cameraFrame.center.x, spawnPosition.x)
        && areClose(renderSnapshot.cameraFrame.center.y, spawnPosition.y)
        && areClose(renderSnapshot.cameraFrame.size.width, 1600.0F / 1.5F)
        && areClose(renderSnapshot.cameraFrame.size.height, 900.0F / 1.5F)
        && debugSnapshot.zoomPercent == 150
        && areClose(playerMarker->position.x, spawnPosition.x)
        && areClose(playerMarker->position.y, spawnPosition.y);
}

[[nodiscard]] bool verifyUpdatedViewportChangesPublishedCameraFrame()
{
    rpg::OverworldRuntime runtime;

    runtime.update(
        0.0F,
        {
            {0.0F, 0.0F},
            {1280.0F, 720.0F},
            {true, 300, false}});
    const rpg::ViewFrame initialFrame = runtime.getRenderSnapshot().cameraFrame;

    runtime.update(
        0.0F,
        {
            {0.0F, 0.0F},
            {1600.0F, 900.0F},
            {true, 300, false}});
    const rpg::ViewFrame resizedFrame = runtime.getRenderSnapshot().cameraFrame;

    return areClose(initialFrame.size.width, 1280.0F / 3.0F)
        && areClose(initialFrame.size.height, 720.0F / 3.0F)
        && areClose(resizedFrame.size.width, 1600.0F / 3.0F)
        && areClose(resizedFrame.size.height, 900.0F / 3.0F);
}

[[nodiscard]] bool verifyRenderSnapshotPublishesGeneratedContent()
{
    rpg::OverworldRuntime runtime;
    rpg::World world;
    runtime.initialize({8192.0F, 8192.0F});

    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const std::vector<rpg::VisibleWorldContent> visibleContent = world.getVisibleContent(renderSnapshot.cameraFrame);

    if (visibleContent.empty() || renderSnapshot.generatedContent.size() != visibleContent.size())
    {
        return false;
    }

    for (const rpg::VisibleWorldContent& content : visibleContent)
    {
        const rpg::OverworldRenderContent* renderContent = findGeneratedContent(
            renderSnapshot.generatedContent,
            content.instance.id);

        if (renderContent == nullptr
            || renderContent->type != content.instance.type
            || renderContent->prototypeId != content.instance.prototypeId
            || renderContent->anchorTile.x != content.instance.anchorTile.x
            || renderContent->anchorTile.y != content.instance.anchorTile.y
            || renderContent->appearanceId.value != content.instance.appearanceId.value
            || !areClose(renderContent->position.x, content.instance.position.x)
            || !areClose(renderContent->position.y, content.instance.position.y)
            || !areClose(renderContent->size.width, content.instance.footprint.size.width)
            || !areClose(renderContent->size.height, content.instance.footprint.size.height)
            || !areClose(renderContent->origin.x, -content.instance.footprint.offset.x)
            || !areClose(renderContent->origin.y, -content.instance.footprint.offset.y)
            || !areClose(renderContent->sortKeyY, content.instance.sortKeyY))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool verifyRenderSnapshotPublishesPixelSnappedCamera()
{
    rpg::OverworldRuntime runtime;
    rpg::World world;
    const rpg::TileCoordinates spawnTile = world.getSpawnTile();
    const std::optional<rpg::TileCoordinates> traversableNeighbor = findAdjacentTile(world, spawnTile, true);

    if (!traversableNeighbor.has_value())
    {
        return false;
    }

    runtime.initialize({320.0F, 224.0F});
    constexpr float kPartialStepSeconds = 2.7F / 96.0F;

    runtime.update(
        kPartialStepSeconds,
        {
            movementIntentForTiles(spawnTile, *traversableNeighbor),
            {320.0F, 224.0F}});

    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const rpg::OverworldRenderMarker* playerMarker = findPlayerMarker(renderSnapshot.markers);

    if (playerMarker == nullptr)
    {
        return false;
    }

    const float pixelStepX = renderSnapshot.cameraFrame.size.width / 320.0F;
    const float pixelStepY = renderSnapshot.cameraFrame.size.height / 224.0F;
    const float expectedSnappedCenterX = std::round(playerMarker->position.x / pixelStepX) * pixelStepX;
    const float expectedSnappedCenterY = std::round(playerMarker->position.y / pixelStepY) * pixelStepY;

    return areClose(renderSnapshot.cameraFrame.center.x, expectedSnappedCenterX)
        && areClose(renderSnapshot.cameraFrame.center.y, expectedSnappedCenterY)
        && (!areClose(renderSnapshot.cameraFrame.center.x, std::round(renderSnapshot.cameraFrame.center.x))
            || !areClose(renderSnapshot.cameraFrame.center.y, std::round(renderSnapshot.cameraFrame.center.y)))
        && containsVisibleTile(renderSnapshot.visibleTiles, world.getTileCoordinates(playerMarker->position));
}

[[nodiscard]] bool verifyDebugSnapshotPublishesOverlayData()
{
    rpg::OverworldRuntime runtime;
    runtime.update(
        0.0F,
        {
            {0.0F, 0.0F},
            {1200.0F, 900.0F},
            {true, 150, false}});

    const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
    const rpg::OverworldDebugSnapshot& debugSnapshot = runtime.getDebugSnapshot();
    rpg::World world;
    world.updateRetentionWindow(renderSnapshot.cameraFrame);
    const std::vector<rpg::VisibleWorldTile> visibleTiles = world.getVisibleTiles(renderSnapshot.cameraFrame);
    const std::vector<rpg::VisibleWorldContent> visibleContent = world.getVisibleContent(renderSnapshot.cameraFrame);

    return !visibleTiles.empty()
        && debugSnapshot.playerTileCoordinates.x == world.getSpawnTile().x
        && debugSnapshot.playerTileCoordinates.y == world.getSpawnTile().y
        && debugSnapshot.zoomPercent == 150
        && debugSnapshot.retainedChunkCount == world.getRetainedChunkCount()
        && debugSnapshot.retainedGeneratedContentCount == world.getRetainedGeneratedContentCount()
        && debugSnapshot.visibleTileCount == visibleTiles.size()
        && debugSnapshot.visibleGeneratedContentCount == visibleContent.size();
}

[[nodiscard]] bool verifyDebugSnapshotReflectsCacheStateAfterRetentionPruning()
{
    rpg::OverworldRuntime runtime;
    rpg::World world;
    rpg::Player player;
    const rpg::TileCoordinates spawnTile = world.getSpawnTile();
    const std::optional<std::vector<rpg::TileCoordinates>> path = findTraversablePathWithMinimumLength(
        world,
        spawnTile,
        96,
        256);

    if (!path.has_value())
    {
        return false;
    }

    const float stepSeconds = world.getTileSize() / player.getMovementSpeed();
    runtime.initialize({320.0F, 224.0F});

    rpg::TileCoordinates currentTile = spawnTile;
    std::size_t maximumRetainedGeneratedContentCount = runtime.getDebugSnapshot().retainedGeneratedContentCount;

    for (const rpg::TileCoordinates& nextTile : *path)
    {
        runtime.update(
            stepSeconds,
            {
                movementIntentForTiles(currentTile, nextTile),
                {320.0F, 224.0F}});
        currentTile = nextTile;

        const rpg::OverworldDebugSnapshot& debugSnapshot = runtime.getDebugSnapshot();
        maximumRetainedGeneratedContentCount = std::max(
            maximumRetainedGeneratedContentCount,
            debugSnapshot.retainedGeneratedContentCount);

        if (debugSnapshot.retainedGeneratedContentCount >= maximumRetainedGeneratedContentCount)
        {
            continue;
        }

        const rpg::OverworldRenderSnapshot& renderSnapshot = runtime.getRenderSnapshot();
        rpg::World verificationWorld;
        const std::vector<rpg::VisibleWorldContent> visibleContent = verificationWorld.getVisibleContent(renderSnapshot.cameraFrame);
        return debugSnapshot.visibleGeneratedContentCount == visibleContent.size();
    }

    return false;
}

} // namespace

int main()
{
    if (!verifySessionInitialization())
    {
        return 1;
    }

    if (!verifyRepeatedInitializationRefreshesPublishedSnapshot())
    {
        return 1;
    }

    if (!verifyGameplayProgression())
    {
        return 1;
    }

    if (!verifyRuntimeCompletesReleasedStep())
    {
        return 1;
    }

    if (!verifyRenderSnapshotContents())
    {
        return 1;
    }

    if (!verifyDebugZoomAdjustsPublishedCameraFrame())
    {
        return 1;
    }

    if (!verifyInitializedRuntimePublishesUpdatedViewportAndZoom())
    {
        return 1;
    }

    if (!verifyUpdatedViewportChangesPublishedCameraFrame())
    {
        return 1;
    }

    if (!verifyRenderSnapshotPublishesGeneratedContent())
    {
        return 1;
    }

    if (!verifyRenderSnapshotPublishesPixelSnappedCamera())
    {
        return 1;
    }

    if (!verifyDebugSnapshotPublishesOverlayData())
    {
        return 1;
    }

    if (!verifyDebugSnapshotReflectsCacheStateAfterRetentionPruning())
    {
        return 1;
    }

    return 0;
}
