/**
 * @file GameRuntimeSupportTests.cpp
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

#include "GameRuntimeSupport.hpp"

#include <cmath>
#include <string>
#include <vector>

namespace
{

[[nodiscard]] bool matchesMovementIntent(
    const rpg::MovementIntent& movementIntent,
    const float expectedX,
    const float expectedY) noexcept
{
    return std::fabs(movementIntent.x - expectedX) < 0.0001F
        && std::fabs(movementIntent.y - expectedY) < 0.0001F;
}

bool verifyCloseSignals()
{
    using rpg::detail::RuntimeEvent;
    using rpg::detail::shouldCloseForEvent;

    return !shouldCloseForEvent(RuntimeEvent::None)
        && shouldCloseForEvent(RuntimeEvent::WindowClosed)
        && shouldCloseForEvent(RuntimeEvent::EscapePressed);
}

bool verifyFrameOrder()
{
    std::vector<std::string> phases;
    float observedDeltaTime = 0.0F;

    rpg::detail::executeFrame(
        [&phases]()
        {
            phases.emplace_back("events");
        },
        [&phases, &observedDeltaTime](const float deltaTimeSeconds)
        {
            phases.emplace_back("update");
            observedDeltaTime = deltaTimeSeconds;
        },
        [&phases]()
        {
            phases.emplace_back("render");
        },
        0.25F);

    return phases == std::vector<std::string>{"events", "update", "render"}
        && std::fabs(observedDeltaTime - 0.25F) < 0.0001F;
}

bool verifyFramePacingConfiguration()
{
    const rpg::detail::WindowFramePacingConfig defaultConfig = rpg::detail::getDefaultWindowFramePacingConfig();
    const rpg::detail::WindowFramePacingConfig fallbackConfig =
        rpg::detail::makeWindowFramePacingConfig(rpg::detail::WindowFramePacingMode::FramerateLimit, 60U);

    return defaultConfig.mode == rpg::detail::WindowFramePacingMode::FramerateLimit
        && defaultConfig.framerateLimit == 60U
        && fallbackConfig.mode == rpg::detail::WindowFramePacingMode::FramerateLimit
        && fallbackConfig.framerateLimit == 60U;
}

bool verifyOverworldRenderPassOrder()
{
    std::vector<std::string> phases;

    rpg::detail::executeOverworldRenderPasses(
        [&phases]()
        {
            phases.emplace_back("terrain");
        },
        [&phases]()
        {
            phases.emplace_back("objects");
        },
        [&phases]()
        {
            phases.emplace_back("tile-grid");
        },
        true);

    return phases == std::vector<std::string>{"terrain", "objects", "tile-grid"};
}

bool verifyOverworldRenderPassOrderWithoutTileGrid()
{
    std::vector<std::string> phases;

    rpg::detail::executeOverworldRenderPasses(
        [&phases]()
        {
            phases.emplace_back("terrain");
        },
        [&phases]()
        {
            phases.emplace_back("objects");
        },
        [&phases]()
        {
            phases.emplace_back("tile-grid");
        },
        false);

    return phases == std::vector<std::string>{"terrain", "objects"};
}

bool verifyPlayerSpritePlacement()
{
    const rpg::WorldPosition playerPosition{144.0F, 208.0F};
    const rpg::detail::PlayerSpritePlacement placement = rpg::detail::getPlayerSpritePlacement(16.0F, playerPosition);

    return std::fabs(placement.size.width - 48.0F) < 0.0001F
        && std::fabs(placement.size.height - 48.0F) < 0.0001F
        && std::fabs(placement.origin.x - 24.0F) < 0.0001F
        && std::fabs(placement.origin.y - 32.0F) < 0.0001F
        && std::fabs(placement.position.x - playerPosition.x) < 0.0001F
        && std::fabs(placement.position.y - playerPosition.y) < 0.0001F;
}

bool verifyOverworldInputTranslation()
{
    const rpg::WorldSize viewportSize{320.0F, 224.0F};
    const rpg::detail::OverworldDirectionalInput canceledHorizontalInput{
        .moveLeft = true,
        .moveRight = true,
        .moveUp = false,
        .moveDown = false};
    const rpg::detail::OverworldDirectionalInput canceledVerticalInput{
        .moveLeft = false,
        .moveRight = false,
        .moveUp = true,
        .moveDown = true};
    rpg::detail::OverworldDirectionalInput lastPressedInput;

    rpg::detail::applyDirectionalInputPress(lastPressedInput, rpg::detail::OverworldDirectionalKey::Right);
    const rpg::MovementIntent rightIntent = rpg::detail::getMovementIntent(lastPressedInput);
    rpg::detail::applyDirectionalInputPress(lastPressedInput, rpg::detail::OverworldDirectionalKey::Up);
    const rpg::MovementIntent upIntent = rpg::detail::getMovementIntent(lastPressedInput);
    rpg::detail::applyDirectionalInputRelease(lastPressedInput, rpg::detail::OverworldDirectionalKey::Up);
    const rpg::MovementIntent backToRightIntent = rpg::detail::getMovementIntent(lastPressedInput);
    rpg::detail::applyDirectionalInputPress(lastPressedInput, rpg::detail::OverworldDirectionalKey::Down);
    const rpg::MovementIntent downIntent = rpg::detail::getMovementIntent(lastPressedInput);

    const rpg::MovementIntent canceledHorizontalIntent = rpg::detail::getMovementIntent(canceledHorizontalInput);
    const rpg::MovementIntent canceledVerticalIntent = rpg::detail::getMovementIntent(canceledVerticalInput);
    const rpg::OverworldInput::DebugViewState debugViewState{
        true,
        250,
        true};
    const rpg::OverworldInput overworldInput = rpg::detail::getOverworldInput(lastPressedInput, viewportSize, debugViewState);

    return matchesMovementIntent(rightIntent, 1.0F, 0.0F)
        && matchesMovementIntent(upIntent, 0.0F, -1.0F)
        && matchesMovementIntent(backToRightIntent, 1.0F, 0.0F)
        && matchesMovementIntent(downIntent, 0.0F, 1.0F)
        && matchesMovementIntent(canceledHorizontalIntent, 0.0F, 0.0F)
        && matchesMovementIntent(canceledVerticalIntent, 0.0F, 0.0F)
        && matchesMovementIntent(overworldInput.movementIntent, downIntent.x, downIntent.y)
        && std::fabs(overworldInput.viewportSize.width - viewportSize.width) < 0.0001F
        && std::fabs(overworldInput.viewportSize.height - viewportSize.height) < 0.0001F
        && overworldInput.debugViewState.isEnabled
        && overworldInput.debugViewState.zoomPercent == 250
        && overworldInput.debugViewState.showTileGrid;
}

bool verifyViewportSizeTranslation()
{
    const rpg::WorldSize viewportSize = rpg::detail::makeViewportSize(1600U, 900U);

    return std::fabs(viewportSize.width - 1600.0F) < 0.0001F
        && std::fabs(viewportSize.height - 900.0F) < 0.0001F;
}

bool verifyDebugViewStateTranslation()
{
    rpg::OverworldInput::DebugViewState debugViewState = rpg::detail::makeOverworldDebugViewState(true);

    rpg::detail::applyOverworldDebugViewAction(debugViewState, rpg::detail::OverworldDebugViewAction::ZoomOut);
    rpg::detail::applyOverworldDebugViewAction(debugViewState, rpg::detail::OverworldDebugViewAction::ZoomOut);
    rpg::detail::applyOverworldDebugViewAction(debugViewState, rpg::detail::OverworldDebugViewAction::ZoomIn);
    rpg::detail::applyOverworldDebugViewAction(debugViewState, rpg::detail::OverworldDebugViewAction::ToggleTileGrid);

    if (!debugViewState.isEnabled
        || debugViewState.zoomPercent != 250
        || !debugViewState.showTileGrid
        || !rpg::detail::shouldRenderTileGridOverlay(debugViewState))
    {
        return false;
    }

    rpg::OverworldInput::DebugViewState disabledDebugViewState = rpg::detail::makeOverworldDebugViewState(false);
    rpg::detail::applyOverworldDebugViewAction(disabledDebugViewState, rpg::detail::OverworldDebugViewAction::ZoomIn);
    rpg::detail::applyOverworldDebugViewAction(disabledDebugViewState, rpg::detail::OverworldDebugViewAction::ToggleTileGrid);

    return !disabledDebugViewState.isEnabled
        && disabledDebugViewState.zoomPercent == 300
        && !disabledDebugViewState.showTileGrid
        && !rpg::detail::shouldRenderTileGridOverlay(disabledDebugViewState);
}

bool verifyDebugOverlayVisibilityTranslation()
{
    rpg::detail::DebugOverlayState debugOverlayState;

    if (rpg::detail::shouldRenderDebugOverlay(debugOverlayState))
    {
        return false;
    }

    rpg::detail::toggleDebugOverlayVisibility(debugOverlayState);

    if (!rpg::detail::shouldRenderDebugOverlay(debugOverlayState))
    {
        return false;
    }

    rpg::detail::toggleDebugOverlayVisibility(debugOverlayState);
    return !rpg::detail::shouldRenderDebugOverlay(debugOverlayState);
}

bool verifyDebugOverlayDoesNotBlockInputTranslation()
{
    const rpg::WorldSize viewportSize{320.0F, 224.0F};
    rpg::detail::DebugOverlayState debugOverlayState;
    rpg::detail::OverworldDirectionalInput directionalInput;
    rpg::OverworldInput::DebugViewState debugViewState = rpg::detail::makeOverworldDebugViewState(true);

    rpg::detail::toggleDebugOverlayVisibility(debugOverlayState);
    rpg::detail::applyDirectionalInputPress(directionalInput, rpg::detail::OverworldDirectionalKey::Left);
    rpg::detail::applyOverworldDebugViewAction(debugViewState, rpg::detail::OverworldDebugViewAction::ZoomOut);
    rpg::detail::applyOverworldDebugViewAction(debugViewState, rpg::detail::OverworldDebugViewAction::ToggleTileGrid);

    const rpg::OverworldInput overworldInput = rpg::detail::getOverworldInput(directionalInput, viewportSize, debugViewState);

    return rpg::detail::shouldRenderDebugOverlay(debugOverlayState)
        && matchesMovementIntent(overworldInput.movementIntent, -1.0F, 0.0F)
        && overworldInput.debugViewState.isEnabled
        && overworldInput.debugViewState.zoomPercent == 250
        && overworldInput.debugViewState.showTileGrid
        && std::fabs(overworldInput.viewportSize.width - viewportSize.width) < 0.0001F
        && std::fabs(overworldInput.viewportSize.height - viewportSize.height) < 0.0001F;
}

bool verifyDebugOverlayStringFormatting()
{
    const rpg::OverworldDebugSnapshot debugSnapshot{
        {12, -7},
        150,
        17,
        42,
        9};

    return rpg::detail::buildDebugOverlayString(debugSnapshot, 144)
        == "FPS: 144\n"
           "Retained chunks: 17\n"
           "Retained objects: 42\n"
           "Rendered objects: 9\n"
           "Coordinates: (12, -7)\n"
           "Zoom: 150%";
}

bool verifyTileGridOverlayGeometry()
{
    const rpg::OverworldRenderTile visibleTile{
        {3, 4},
        rpg::TileType::Grass,
        {16.0F, 16.0F},
        {8.0F, 8.0F},
        {56.0F, 72.0F}};
    const auto overlayRectangles = rpg::detail::getTileGridOverlayRectangles(visibleTile, 1.0F);

    return std::fabs(overlayRectangles[0].position.x - 48.0F) < 0.0001F
        && std::fabs(overlayRectangles[0].position.y - 64.0F) < 0.0001F
        && std::fabs(overlayRectangles[0].size.width - 16.0F) < 0.0001F
        && std::fabs(overlayRectangles[1].size.height - 16.0F) < 0.0001F
        && std::fabs(overlayRectangles[2].position.y - 79.0F) < 0.0001F
        && std::fabs(overlayRectangles[3].position.x - 63.0F) < 0.0001F;
}

bool verifyOverworldRenderOrdering()
{
    const rpg::OverworldRenderContent higherVegetation{
        17,
        rpg::ContentType::Tree,
        "oak_tree_large_dark_1",
        {4, 5},
        {96.0F, 128.0F},
        {56.0F, 120.0F},
        {72.0F, 88.0F},
        {11},
        88.0F};
    const rpg::OverworldRenderContent lowerVegetation{
        18,
        rpg::ContentType::Shrub,
        "bush_small_1",
        {4, 8},
        {32.0F, 32.0F},
        {8.0F, 24.0F},
        {72.0F, 136.0F},
        {19},
        136.0F};
    const rpg::OverworldRenderMarker playerMarker{
        {48.0F, 48.0F},
        {24.0F, 32.0F},
        {72.0F, 136.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Down,
        1,
        136.0F};

    return rpg::detail::shouldRenderBefore(
               rpg::detail::makeRenderOrderKey(higherVegetation),
               rpg::detail::makeRenderOrderKey(lowerVegetation))
        && rpg::detail::shouldRenderBefore(
            rpg::detail::makeRenderOrderKey(lowerVegetation),
            rpg::detail::makeRenderOrderKey(playerMarker))
        && !rpg::detail::shouldRenderBefore(
            rpg::detail::makeRenderOrderKey(playerMarker),
            rpg::detail::makeRenderOrderKey(lowerVegetation));
}

bool verifyPixelSnappedViewFrame()
{
    const rpg::ViewFrame unsnappedFrame{{103.2F, 135.6F}, {320.0F, 180.0F}};
    const rpg::ViewFrame snappedFrame = rpg::detail::snapViewFrameToPixelGrid(unsnappedFrame, {960.0F, 540.0F});

    return std::fabs(snappedFrame.center.x - (310.0F / 3.0F)) < 0.0001F
        && std::fabs(snappedFrame.center.y - (407.0F / 3.0F)) < 0.0001F
        && std::fabs(snappedFrame.size.width - unsnappedFrame.size.width) < 0.0001F
        && std::fabs(snappedFrame.size.height - unsnappedFrame.size.height) < 0.0001F;
}

} // namespace

int main()
{
    if (!verifyCloseSignals())
    {
        return 1;
    }

    if (!verifyFrameOrder())
    {
        return 1;
    }

    if (!verifyFramePacingConfiguration())
    {
        return 1;
    }

    if (!verifyOverworldRenderPassOrder())
    {
        return 1;
    }

    if (!verifyOverworldRenderPassOrderWithoutTileGrid())
    {
        return 1;
    }

    if (!verifyPlayerSpritePlacement())
    {
        return 1;
    }

    if (!verifyOverworldInputTranslation())
    {
        return 1;
    }

    if (!verifyViewportSizeTranslation())
    {
        return 1;
    }

    if (!verifyDebugViewStateTranslation())
    {
        return 1;
    }

    if (!verifyDebugOverlayVisibilityTranslation())
    {
        return 1;
    }

    if (!verifyDebugOverlayDoesNotBlockInputTranslation())
    {
        return 1;
    }

    if (!verifyDebugOverlayStringFormatting())
    {
        return 1;
    }

    if (!verifyTileGridOverlayGeometry())
    {
        return 1;
    }

    if (!verifyOverworldRenderOrdering())
    {
        return 1;
    }

    if (!verifyPixelSnappedViewFrame())
    {
        return 1;
    }

    return 0;
}
