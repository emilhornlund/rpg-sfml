/**
 * @file GameRuntimeSupport.hpp
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

#ifndef RPG_MAIN_GAME_RUNTIME_SUPPORT_HPP
#define RPG_MAIN_GAME_RUNTIME_SUPPORT_HPP

#include <main/OverworldRuntime.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace rpg::detail
{

enum class RuntimeEvent
{
    None,
    WindowClosed,
    EscapePressed
};

struct PlayerSpritePlacement
{
    WorldSize size{0.0F, 0.0F};
    WorldPosition origin{0.0F, 0.0F};
    WorldPosition position{0.0F, 0.0F};
};

struct OverworldDirectionalInput
{
    bool moveLeft = false;
    bool moveRight = false;
    bool moveUp = false;
    bool moveDown = false;
    std::uint64_t horizontalPressSequence = 0;
    std::uint64_t verticalPressSequence = 0;
    std::uint64_t nextPressSequence = 1;
};

enum class OverworldDirectionalKey
{
    Left,
    Right,
    Up,
    Down
};

enum class OverworldDebugViewAction
{
    ZoomIn,
    ZoomOut,
    ToggleTileGrid
};

struct OverlayRectangle
{
    WorldSize size{0.0F, 0.0F};
    WorldPosition position{0.0F, 0.0F};
};

enum class WindowFramePacingMode
{
    VerticalSync,
    FramerateLimit
};

struct WindowFramePacingConfig
{
    WindowFramePacingMode mode = WindowFramePacingMode::FramerateLimit;
    unsigned int framerateLimit = 0;
};

struct DebugOverlayState
{
    bool isVisible = false;
};

struct DebugOverlayRenderMetrics
{
    std::size_t frontOccluderCount = 0;
    std::size_t overlapQualifiedOcclusionCandidateCount = 0;
    std::size_t terrainVertexCount = 0;
    std::size_t gridVertexCount = 0;
};

struct WorldRectangle
{
    WorldPosition position{0.0F, 0.0F};
    WorldSize size{0.0F, 0.0F};
};

constexpr int kMinDebugZoomPercent = 100;
constexpr int kDefaultDebugZoomPercent = 300;
constexpr int kMaxDebugZoomPercent = 400;
constexpr int kDebugZoomStepPercent = 50;

[[nodiscard]] constexpr bool isDebugViewModeEnabledForBuild() noexcept
{
#ifdef NDEBUG
    return false;
#else
    return true;
#endif
}

[[nodiscard]] constexpr OverworldInput::DebugViewState makeOverworldDebugViewState(const bool isEnabled) noexcept
{
    return {
        isEnabled,
        kDefaultDebugZoomPercent,
        false};
}

constexpr void applyOverworldDebugViewAction(
    OverworldInput::DebugViewState& debugViewState,
    const OverworldDebugViewAction action) noexcept
{
    if (!debugViewState.isEnabled)
    {
        return;
    }

    switch (action)
    {
    case OverworldDebugViewAction::ZoomIn:
        debugViewState.zoomPercent = debugViewState.zoomPercent + kDebugZoomStepPercent > kMaxDebugZoomPercent
            ? kMaxDebugZoomPercent
            : debugViewState.zoomPercent + kDebugZoomStepPercent;
        break;
    case OverworldDebugViewAction::ZoomOut:
        debugViewState.zoomPercent = debugViewState.zoomPercent - kDebugZoomStepPercent < kMinDebugZoomPercent
            ? kMinDebugZoomPercent
            : debugViewState.zoomPercent - kDebugZoomStepPercent;
        break;
    case OverworldDebugViewAction::ToggleTileGrid:
        debugViewState.showTileGrid = !debugViewState.showTileGrid;
        break;
    }
}

[[nodiscard]] constexpr bool shouldCloseForEvent(const RuntimeEvent event) noexcept
{
    return event == RuntimeEvent::WindowClosed || event == RuntimeEvent::EscapePressed;
}

constexpr void applyDirectionalInputPress(
    OverworldDirectionalInput& directionalInput,
    const OverworldDirectionalKey directionalKey) noexcept
{
    switch (directionalKey)
    {
    case OverworldDirectionalKey::Left:
        if (!directionalInput.moveLeft)
        {
            directionalInput.moveLeft = true;
            directionalInput.horizontalPressSequence = directionalInput.nextPressSequence++;
        }
        break;
    case OverworldDirectionalKey::Right:
        if (!directionalInput.moveRight)
        {
            directionalInput.moveRight = true;
            directionalInput.horizontalPressSequence = directionalInput.nextPressSequence++;
        }
        break;
    case OverworldDirectionalKey::Up:
        if (!directionalInput.moveUp)
        {
            directionalInput.moveUp = true;
            directionalInput.verticalPressSequence = directionalInput.nextPressSequence++;
        }
        break;
    case OverworldDirectionalKey::Down:
        if (!directionalInput.moveDown)
        {
            directionalInput.moveDown = true;
            directionalInput.verticalPressSequence = directionalInput.nextPressSequence++;
        }
        break;
    }
}

constexpr void applyDirectionalInputRelease(
    OverworldDirectionalInput& directionalInput,
    const OverworldDirectionalKey directionalKey) noexcept
{
    switch (directionalKey)
    {
    case OverworldDirectionalKey::Left:
        directionalInput.moveLeft = false;
        break;
    case OverworldDirectionalKey::Right:
        directionalInput.moveRight = false;
        break;
    case OverworldDirectionalKey::Up:
        directionalInput.moveUp = false;
        break;
    case OverworldDirectionalKey::Down:
        directionalInput.moveDown = false;
        break;
    }
}

[[nodiscard]] constexpr MovementIntent getMovementIntent(const OverworldDirectionalInput& directionalInput) noexcept
{
    const float horizontalIntent = directionalInput.moveLeft == directionalInput.moveRight
        ? 0.0F
        : (directionalInput.moveRight ? 1.0F : -1.0F);
    const float verticalIntent = directionalInput.moveUp == directionalInput.moveDown
        ? 0.0F
        : (directionalInput.moveDown ? 1.0F : -1.0F);

    if (horizontalIntent != 0.0F && verticalIntent != 0.0F)
    {
        if (directionalInput.verticalPressSequence > directionalInput.horizontalPressSequence)
        {
            return {0.0F, verticalIntent};
        }

        return {horizontalIntent, 0.0F};
    }

    return {horizontalIntent, verticalIntent};
}

[[nodiscard]] constexpr OverworldInput getOverworldInput(
    const OverworldDirectionalInput& directionalInput,
    const WorldSize& viewportSize,
    const OverworldInput::DebugViewState& debugViewState) noexcept
{
    return {
        getMovementIntent(directionalInput),
        viewportSize,
        debugViewState};
}

[[nodiscard]] constexpr WorldSize makeViewportSize(
    const std::uint32_t widthPixels,
    const std::uint32_t heightPixels) noexcept
{
    return {
        static_cast<float>(widthPixels),
        static_cast<float>(heightPixels)};
}

[[nodiscard]] constexpr ViewFrame makeScreenSpaceViewFrame(
    const std::uint32_t widthPixels,
    const std::uint32_t heightPixels) noexcept
{
    const WorldSize size = makeViewportSize(widthPixels, heightPixels);
    return {
        {size.width * 0.5F, size.height * 0.5F},
        size};
}

[[nodiscard]] constexpr bool shouldRenderTileGridOverlay(const OverworldInput::DebugViewState& debugViewState) noexcept
{
    return debugViewState.isEnabled && debugViewState.showTileGrid;
}

constexpr void toggleDebugOverlayVisibility(DebugOverlayState& debugOverlayState) noexcept
{
    debugOverlayState.isVisible = !debugOverlayState.isVisible;
}

[[nodiscard]] constexpr bool shouldRenderDebugOverlay(const DebugOverlayState& debugOverlayState) noexcept
{
    return debugOverlayState.isVisible;
}

[[nodiscard]] std::string buildDebugOverlayString(
    const OverworldDebugSnapshot& debugSnapshot,
    const DebugOverlayRenderMetrics& renderMetrics,
    const int displayedFramesPerSecond);

[[nodiscard]] constexpr WindowFramePacingConfig makeWindowFramePacingConfig(
    const WindowFramePacingMode mode,
    const unsigned int framerateLimit = 0) noexcept
{
    return {
        mode,
        mode == WindowFramePacingMode::FramerateLimit ? framerateLimit : 0U};
}

[[nodiscard]] constexpr WindowFramePacingConfig getDefaultWindowFramePacingConfig() noexcept
{
    return makeWindowFramePacingConfig(WindowFramePacingMode::FramerateLimit, 60U);
}

[[nodiscard]] ViewFrame snapViewFrameToPixelGrid(
    const ViewFrame& frame,
    const WorldSize& viewportSizeInPixels) noexcept;

[[nodiscard]] constexpr std::array<OverlayRectangle, 4> getTileGridOverlayRectangles(
    const OverworldRenderTile& visibleTile,
    const float lineThickness) noexcept
{
    const float clampedLineThickness = lineThickness > 0.0F ? lineThickness : 1.0F;
    const float left = visibleTile.position.x - visibleTile.origin.x;
    const float top = visibleTile.position.y - visibleTile.origin.y;
    const float width = visibleTile.size.width;
    const float height = visibleTile.size.height;
    const float bottom = top + height - clampedLineThickness;
    const float right = left + width - clampedLineThickness;

    return {{
        {{width, clampedLineThickness}, {left, top}},
        {{clampedLineThickness, height}, {left, top}},
        {{width, clampedLineThickness}, {left, bottom}},
        {{clampedLineThickness, height}, {right, top}},
    }};
}

[[nodiscard]] constexpr PlayerSpritePlacement getPlayerSpritePlacement(
    const float tileSize,
    const WorldPosition& playerPosition) noexcept
{
    const float spriteFrameSize = tileSize * 3.0F;
    return {
        {spriteFrameSize, spriteFrameSize},
        {spriteFrameSize * 0.5F, tileSize * 2.0F},
        playerPosition};
}

[[nodiscard]] constexpr WorldRectangle makeWorldRectangle(
    const WorldPosition& position,
    const WorldSize& size,
    const WorldPosition& origin) noexcept
{
    return {
        {position.x - origin.x, position.y - origin.y},
        size};
}

[[nodiscard]] constexpr WorldRectangle getWorldRectangle(const OverworldRenderContent& renderContent) noexcept
{
    return makeWorldRectangle(renderContent.position, renderContent.size, renderContent.origin);
}

[[nodiscard]] constexpr WorldRectangle getWorldRectangle(const OverworldRenderMarker& renderMarker) noexcept
{
    return makeWorldRectangle(renderMarker.position, renderMarker.size, renderMarker.origin);
}

[[nodiscard]] constexpr bool doWorldRectanglesIntersect(
    const WorldRectangle& lhs,
    const WorldRectangle& rhs) noexcept
{
    if (lhs.size.width <= 0.0F || lhs.size.height <= 0.0F || rhs.size.width <= 0.0F || rhs.size.height <= 0.0F)
    {
        return false;
    }

    const float lhsRight = lhs.position.x + lhs.size.width;
    const float lhsBottom = lhs.position.y + lhs.size.height;
    const float rhsRight = rhs.position.x + rhs.size.width;
    const float rhsBottom = rhs.position.y + rhs.size.height;

    return lhs.position.x <= rhsRight
        && lhsRight >= rhs.position.x
        && lhs.position.y <= rhsBottom
        && lhsBottom >= rhs.position.y;
}

template <typename ProcessEventsFn, typename UpdateFn, typename RenderFn>
void executeFrame(
    ProcessEventsFn&& processEvents,
    UpdateFn&& update,
    RenderFn&& render,
    const float deltaTimeSeconds)
{
    std::forward<ProcessEventsFn>(processEvents)();
    std::forward<UpdateFn>(update)(deltaTimeSeconds);
    std::forward<RenderFn>(render)();
}

template <typename ApplyViewFn, typename RenderWorldFn>
void executeViewFramedRender(
    ApplyViewFn&& applyView,
    RenderWorldFn&& renderWorld)
{
    std::forward<ApplyViewFn>(applyView)();
    std::forward<RenderWorldFn>(renderWorld)();
}

template <typename RenderTerrainFn, typename RenderObjectsFn, typename RenderTileGridFn>
void executeOverworldRenderPasses(
    RenderTerrainFn&& renderTerrain,
    RenderObjectsFn&& renderObjects,
    RenderTileGridFn&& renderTileGrid,
    const bool shouldRenderTileGrid)
{
    std::forward<RenderTerrainFn>(renderTerrain)();
    std::forward<RenderObjectsFn>(renderObjects)();

    if (shouldRenderTileGrid)
    {
        std::forward<RenderTileGridFn>(renderTileGrid)();
    }
}

struct OverworldRenderOrderKey
{
    float sortKeyY = 0.0F;
    std::uint8_t kindPriority = 0;
    std::uint64_t stableId = 0;
};

enum class OverworldRenderQueueEntryKind : std::uint8_t
{
    GeneratedContent = 0,
    PlayerMarker = 1
};

struct OverworldRenderQueueEntry
{
    OverworldRenderQueueEntryKind kind = OverworldRenderQueueEntryKind::GeneratedContent;
    OverworldRenderOrderKey orderKey{};
    std::size_t sourceIndex = 0;
};

[[nodiscard]] constexpr OverworldRenderOrderKey makeRenderOrderKey(const OverworldRenderContent& renderContent) noexcept
{
    return {renderContent.sortKeyY, 0U, renderContent.id};
}

[[nodiscard]] constexpr OverworldRenderOrderKey makeRenderOrderKey(const OverworldRenderMarker& renderMarker) noexcept
{
    return {renderMarker.sortKeyY, 1U, std::numeric_limits<std::uint64_t>::max()};
}

[[nodiscard]] constexpr OverworldRenderQueueEntry makeRenderQueueEntry(
    const OverworldRenderContent& renderContent,
    const std::size_t sourceIndex) noexcept
{
    return {OverworldRenderQueueEntryKind::GeneratedContent, makeRenderOrderKey(renderContent), sourceIndex};
}

[[nodiscard]] constexpr OverworldRenderQueueEntry makeRenderQueueEntry(
    const OverworldRenderMarker& renderMarker,
    const std::size_t sourceIndex) noexcept
{
    return {OverworldRenderQueueEntryKind::PlayerMarker, makeRenderOrderKey(renderMarker), sourceIndex};
}

[[nodiscard]] constexpr bool shouldRenderBefore(
    const OverworldRenderOrderKey& lhs,
    const OverworldRenderOrderKey& rhs) noexcept
{
    if (lhs.sortKeyY != rhs.sortKeyY)
    {
        return lhs.sortKeyY < rhs.sortKeyY;
    }

    if (lhs.kindPriority != rhs.kindPriority)
    {
        return lhs.kindPriority < rhs.kindPriority;
    }

    return lhs.stableId < rhs.stableId;
}

[[nodiscard]] std::vector<std::size_t> collectFrontGeneratedContentIndices(
    const std::vector<OverworldRenderQueueEntry>& renderQueue);

[[nodiscard]] std::vector<std::size_t> collectOverlapQualifiedFrontGeneratedContentIndices(
    const std::vector<std::size_t>& frontGeneratedContentIndices,
    const std::vector<OverworldRenderContent>& generatedContent,
    const OverworldRenderMarker& playerMarker);

} // namespace rpg::detail

#endif // RPG_MAIN_GAME_RUNTIME_SUPPORT_HPP
