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

bool verifyPlayerMarkerPlacement()
{
    const rpg::WorldPosition playerPosition{144.0F, 208.0F};
    const rpg::detail::PlayerMarkerPlacement placement = rpg::detail::getPlayerMarkerPlacement(32.0F, playerPosition);

    return std::fabs(placement.size.width - 16.0F) < 0.0001F
        && std::fabs(placement.size.height - 16.0F) < 0.0001F
        && std::fabs(placement.origin.x - 8.0F) < 0.0001F
        && std::fabs(placement.origin.y - 8.0F) < 0.0001F
        && std::fabs(placement.position.x - playerPosition.x) < 0.0001F
        && std::fabs(placement.position.y - playerPosition.y) < 0.0001F;
}

bool verifyOverworldInputTranslation()
{
    const rpg::WorldSize viewportSize{320.0F, 224.0F};
    const rpg::detail::OverworldDirectionalInput diagonalInput{
        .moveLeft = false,
        .moveRight = true,
        .moveUp = true,
        .moveDown = false};
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
    const rpg::MovementIntent diagonalIntent = rpg::detail::getMovementIntent(diagonalInput);
    const rpg::MovementIntent canceledHorizontalIntent = rpg::detail::getMovementIntent(canceledHorizontalInput);
    const rpg::MovementIntent canceledVerticalIntent = rpg::detail::getMovementIntent(canceledVerticalInput);
    const rpg::OverworldInput overworldInput = rpg::detail::getOverworldInput(diagonalInput, viewportSize);

    return std::fabs(diagonalIntent.x - 1.0F) < 0.0001F
        && std::fabs(diagonalIntent.y + 1.0F) < 0.0001F
        && std::fabs(canceledHorizontalIntent.x) < 0.0001F
        && std::fabs(canceledHorizontalIntent.y) < 0.0001F
        && std::fabs(canceledVerticalIntent.x) < 0.0001F
        && std::fabs(canceledVerticalIntent.y) < 0.0001F
        && std::fabs(overworldInput.movementIntent.x - diagonalIntent.x) < 0.0001F
        && std::fabs(overworldInput.movementIntent.y - diagonalIntent.y) < 0.0001F
        && std::fabs(overworldInput.viewportSize.width - viewportSize.width) < 0.0001F
        && std::fabs(overworldInput.viewportSize.height - viewportSize.height) < 0.0001F;
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

    if (!verifyPlayerMarkerPlacement())
    {
        return 1;
    }

    if (!verifyOverworldInputTranslation())
    {
        return 1;
    }

    return 0;
}
