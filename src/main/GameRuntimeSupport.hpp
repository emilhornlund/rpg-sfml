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

#include <main/OverworldTypes.hpp>

#include <utility>

namespace rpg::detail
{

enum class RuntimeEvent
{
    None,
    WindowClosed,
    EscapePressed
};

struct PlayerMarkerPlacement
{
    WorldSize size{0.0F, 0.0F};
    WorldPosition origin{0.0F, 0.0F};
    WorldPosition position{0.0F, 0.0F};
};

[[nodiscard]] constexpr bool shouldCloseForEvent(const RuntimeEvent event) noexcept
{
    return event == RuntimeEvent::WindowClosed || event == RuntimeEvent::EscapePressed;
}

[[nodiscard]] constexpr PlayerMarkerPlacement getPlayerMarkerPlacement(
    const float tileSize,
    const WorldPosition& playerPosition) noexcept
{
    const float markerSize = tileSize * 0.5F;
    return {
        {markerSize, markerSize},
        {markerSize * 0.5F, markerSize * 0.5F},
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
