/**
 * @file GameEventSupport.hpp
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

#ifndef RPG_MAIN_GAME_EVENT_SUPPORT_HPP
#define RPG_MAIN_GAME_EVENT_SUPPORT_HPP

#include "GameDebugOverlaySupport.hpp"
#include "GameRuntimeSupport.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>

namespace rpg::detail
{

struct GameEventRuntimeState
{
    sf::View& overlayView;
    OverworldDirectionalInput& directionalInput;
    OverworldInput::DebugViewState& debugViewState;
    DebugOverlayRuntime& debugOverlayRuntime;
};

void updateScreenSpaceOverlayView(sf::View& overlayView, sf::Vector2u size);

[[nodiscard]] RuntimeEvent applyGameEvent(const sf::Event& event, GameEventRuntimeState runtimeState) noexcept;

[[nodiscard]] bool processGameEvents(sf::RenderWindow& window, GameEventRuntimeState runtimeState);

} // namespace rpg::detail

#endif // RPG_MAIN_GAME_EVENT_SUPPORT_HPP
