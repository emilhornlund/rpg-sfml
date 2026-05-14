/**
 * @file GameEventSupport.cpp
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

#include "GameEventSupport.hpp"

#include "GameRenderSupport.hpp"

#include <optional>

namespace rpg
{
namespace detail
{
namespace
{

[[nodiscard]] std::optional<OverworldDirectionalKey> getDirectionalKey(const sf::Keyboard::Key key) noexcept
{
    switch (key)
    {
    case sf::Keyboard::Key::A:
    case sf::Keyboard::Key::Left:
        return OverworldDirectionalKey::Left;
    case sf::Keyboard::Key::D:
    case sf::Keyboard::Key::Right:
        return OverworldDirectionalKey::Right;
    case sf::Keyboard::Key::W:
    case sf::Keyboard::Key::Up:
        return OverworldDirectionalKey::Up;
    case sf::Keyboard::Key::S:
    case sf::Keyboard::Key::Down:
        return OverworldDirectionalKey::Down;
    default:
        return std::nullopt;
    }
}

[[nodiscard]] std::optional<OverworldDebugViewAction> getDebugViewAction(const sf::Keyboard::Key key) noexcept
{
    switch (key)
    {
    case sf::Keyboard::Key::Q:
        return OverworldDebugViewAction::ZoomOut;
    case sf::Keyboard::Key::E:
        return OverworldDebugViewAction::ZoomIn;
    case sf::Keyboard::Key::G:
        return OverworldDebugViewAction::ToggleTileGrid;
    default:
        return std::nullopt;
    }
}

} // namespace

void updateScreenSpaceOverlayView(sf::View& overlayView, const sf::Vector2u size)
{
    applyViewFrame(overlayView, makeScreenSpaceViewFrame(size.x, size.y));
}

RuntimeEvent applyGameEvent(const sf::Event& event, const GameEventRuntimeState runtimeState) noexcept
{
    RuntimeEvent runtimeEvent = RuntimeEvent::None;

    if (event.is<sf::Event::Closed>())
    {
        runtimeEvent = RuntimeEvent::WindowClosed;
    }
    else if (const auto* resized = event.getIf<sf::Event::Resized>())
    {
        updateScreenSpaceOverlayView(runtimeState.overlayView, resized->size);
    }
    else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (const std::optional directionalKey = getDirectionalKey(keyPressed->code); directionalKey.has_value())
        {
            applyDirectionalInputPress(runtimeState.directionalInput, *directionalKey);
        }

        if (const std::optional debugViewAction = getDebugViewAction(keyPressed->code); debugViewAction.has_value())
        {
            applyOverworldDebugViewAction(runtimeState.debugViewState, *debugViewAction);
        }

        if (keyPressed->code == sf::Keyboard::Key::F1)
        {
            toggleDebugOverlayVisibility(runtimeState.debugOverlayRuntime.state);
        }

        if (keyPressed->code == sf::Keyboard::Key::Escape)
        {
            runtimeEvent = RuntimeEvent::EscapePressed;
        }
    }
    else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>())
    {
        if (const std::optional directionalKey = getDirectionalKey(keyReleased->code); directionalKey.has_value())
        {
            applyDirectionalInputRelease(runtimeState.directionalInput, *directionalKey);
        }
    }

    return runtimeEvent;
}

bool processGameEvents(sf::RenderWindow& window, const GameEventRuntimeState runtimeState)
{
    bool shouldCloseWindow = false;

    while (const std::optional event = window.pollEvent())
    {
        shouldCloseWindow = shouldCloseWindow || shouldCloseForEvent(applyGameEvent(*event, runtimeState));
    }

    return shouldCloseWindow;
}

} // namespace detail
} // namespace rpg
