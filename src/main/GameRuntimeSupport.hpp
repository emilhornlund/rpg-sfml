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

#include <cstdint>
#include <utility>

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
    const WorldSize& viewportSize) noexcept
{
    return {
        getMovementIntent(directionalInput),
        viewportSize};
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

} // namespace rpg::detail

#endif // RPG_MAIN_GAME_RUNTIME_SUPPORT_HPP
