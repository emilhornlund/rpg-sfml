/**
 * @file GameEventSupportTests.cpp
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

#include <cmath>

namespace
{

[[nodiscard]] rpg::detail::GameEventRuntimeState makeRuntimeState(
    sf::View& overlayView,
    rpg::detail::OverworldDirectionalInput& directionalInput,
    rpg::OverworldInput::DebugViewState& debugViewState,
    rpg::detail::DebugOverlayRuntime& debugOverlayRuntime)
{
    return {overlayView, directionalInput, debugViewState, debugOverlayRuntime};
}

[[nodiscard]] bool verifyResizeEventTranslation()
{
    sf::View overlayView;
    rpg::detail::OverworldDirectionalInput directionalInput;
    rpg::OverworldInput::DebugViewState debugViewState = rpg::detail::makeOverworldDebugViewState(true);
    rpg::detail::DebugOverlayRuntime debugOverlayRuntime;
    const sf::Event resizedEvent = sf::Event::Resized{{640U, 360U}};

    if (rpg::detail::applyGameEvent(
            resizedEvent,
            makeRuntimeState(overlayView, directionalInput, debugViewState, debugOverlayRuntime))
        != rpg::detail::RuntimeEvent::None)
    {
        return false;
    }

    const sf::Vector2f center = overlayView.getCenter();
    const sf::Vector2f size = overlayView.getSize();

    return std::fabs(center.x - 320.0F) < 0.0001F
        && std::fabs(center.y - 180.0F) < 0.0001F
        && std::fabs(size.x - 640.0F) < 0.0001F
        && std::fabs(size.y - 360.0F) < 0.0001F;
}

[[nodiscard]] bool verifyKeyPressTranslation()
{
    sf::View overlayView;
    rpg::detail::OverworldDirectionalInput directionalInput;
    rpg::OverworldInput::DebugViewState debugViewState = rpg::detail::makeOverworldDebugViewState(true);
    rpg::detail::DebugOverlayRuntime debugOverlayRuntime;
    const sf::Event moveRightPressed = sf::Event::KeyPressed{sf::Keyboard::Key::D, {}, false, false, false, false};
    const sf::Event zoomOutPressed = sf::Event::KeyPressed{sf::Keyboard::Key::Q, {}, false, false, false, false};
    const sf::Event toggleGridPressed = sf::Event::KeyPressed{sf::Keyboard::Key::G, {}, false, false, false, false};
    const sf::Event toggleOverlayPressed = sf::Event::KeyPressed{sf::Keyboard::Key::F1, {}, false, false, false, false};
    const sf::Event escapePressed = sf::Event::KeyPressed{sf::Keyboard::Key::Escape, {}, false, false, false, false};

    const auto runtimeState = makeRuntimeState(overlayView, directionalInput, debugViewState, debugOverlayRuntime);

    if (rpg::detail::applyGameEvent(moveRightPressed, runtimeState) != rpg::detail::RuntimeEvent::None
        || !directionalInput.moveRight)
    {
        return false;
    }

    if (rpg::detail::applyGameEvent(zoomOutPressed, runtimeState) != rpg::detail::RuntimeEvent::None
        || rpg::detail::applyGameEvent(toggleGridPressed, runtimeState) != rpg::detail::RuntimeEvent::None
        || rpg::detail::applyGameEvent(toggleOverlayPressed, runtimeState) != rpg::detail::RuntimeEvent::None)
    {
        return false;
    }

    return debugViewState.zoomPercent == 250
        && debugViewState.showTileGrid
        && rpg::detail::shouldRenderDebugOverlay(debugOverlayRuntime.state)
        && rpg::detail::applyGameEvent(escapePressed, runtimeState) == rpg::detail::RuntimeEvent::EscapePressed;
}

[[nodiscard]] bool verifyKeyReleaseTranslation()
{
    sf::View overlayView;
    rpg::detail::OverworldDirectionalInput directionalInput;
    directionalInput.moveLeft = true;
    rpg::OverworldInput::DebugViewState debugViewState = rpg::detail::makeOverworldDebugViewState(true);
    rpg::detail::DebugOverlayRuntime debugOverlayRuntime;
    const sf::Event moveLeftReleased = sf::Event::KeyReleased{sf::Keyboard::Key::A, {}, false, false, false, false};

    return rpg::detail::applyGameEvent(
               moveLeftReleased,
               makeRuntimeState(overlayView, directionalInput, debugViewState, debugOverlayRuntime))
            == rpg::detail::RuntimeEvent::None
        && !directionalInput.moveLeft;
}

[[nodiscard]] bool verifyCloseEventTranslation()
{
    sf::View overlayView;
    rpg::detail::OverworldDirectionalInput directionalInput;
    rpg::OverworldInput::DebugViewState debugViewState = rpg::detail::makeOverworldDebugViewState(true);
    rpg::detail::DebugOverlayRuntime debugOverlayRuntime;
    const sf::Event closedEvent = sf::Event::Closed{};

    return rpg::detail::applyGameEvent(
               closedEvent,
               makeRuntimeState(overlayView, directionalInput, debugViewState, debugOverlayRuntime))
        == rpg::detail::RuntimeEvent::WindowClosed;
}

} // namespace

int main()
{
    if (!verifyResizeEventTranslation())
    {
        return 1;
    }

    if (!verifyKeyPressTranslation())
    {
        return 1;
    }

    if (!verifyKeyReleaseTranslation())
    {
        return 1;
    }

    if (!verifyCloseEventTranslation())
    {
        return 1;
    }

    return 0;
}
