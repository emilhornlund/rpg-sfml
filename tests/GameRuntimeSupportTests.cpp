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
    const rpg::OverworldInput overworldInput = rpg::detail::getOverworldInput(lastPressedInput, viewportSize);

    return matchesMovementIntent(rightIntent, 1.0F, 0.0F)
        && matchesMovementIntent(upIntent, 0.0F, -1.0F)
        && matchesMovementIntent(backToRightIntent, 1.0F, 0.0F)
        && matchesMovementIntent(downIntent, 0.0F, 1.0F)
        && matchesMovementIntent(canceledHorizontalIntent, 0.0F, 0.0F)
        && matchesMovementIntent(canceledVerticalIntent, 0.0F, 0.0F)
        && matchesMovementIntent(overworldInput.movementIntent, downIntent.x, downIntent.y)
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

    if (!verifyPlayerSpritePlacement())
    {
        return 1;
    }

    if (!verifyOverworldInputTranslation())
    {
        return 1;
    }

    return 0;
}
