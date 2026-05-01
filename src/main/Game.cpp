/**
 * @file Game.cpp
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

#include <main/Game.hpp>
#include <main/OverworldRuntime.hpp>

#include "GameRuntimeSupport.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>

#include <optional>
#include <utility>

namespace rpg
{
constexpr unsigned int kWindowWidth = 1280;
constexpr unsigned int kWindowHeight = 720;
const sf::Color kBackgroundColor(24, 24, 27);
const sf::Color kGrassColor(74, 138, 72);
const sf::Color kSandColor(196, 182, 112);
const sf::Color kWaterColor(48, 102, 190);
const sf::Color kForestColor(39, 92, 46);
const sf::Color kPlayerColor(231, 231, 236);

[[nodiscard]] MovementIntent readMovementIntent() noexcept
{
    MovementIntent movementIntent{0.0F, 0.0F};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        movementIntent.x -= 1.0F;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        movementIntent.x += 1.0F;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
    {
        movementIntent.y -= 1.0F;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
    {
        movementIntent.y += 1.0F;
    }

    return movementIntent;
}

[[nodiscard]] sf::Color getTileColor(const TileType tileType) noexcept
{
    switch (tileType)
    {
    case TileType::Grass:
        return kGrassColor;
    case TileType::Sand:
        return kSandColor;
    case TileType::Forest:
        return kForestColor;
    case TileType::Water:
        return kWaterColor;
    }

    return kBackgroundColor;
}

class Game::Impl
{
public:
    Impl()
        : window(sf::VideoMode({kWindowWidth, kWindowHeight}), "rpg-sfml", sf::State::Windowed)
    {
        window.setFramerateLimit(60);
    }

    sf::RenderWindow window;
    OverworldRuntime overworldRuntime;
};

Game::Game()
    : m_impl(std::make_unique<Impl>())
{
}

Game::~Game() = default;

Game::Game(Game&&) noexcept = default;

Game& Game::operator=(Game&&) noexcept = default;

int Game::run()
{
    sf::Clock frameClock;

    while (m_impl->window.isOpen())
    {
        const float deltaTimeSeconds = frameClock.restart().asSeconds();

        detail::executeFrame(
            [this]()
            {
                processEvents();
            },
            [this](const float frameDeltaTimeSeconds)
            {
                update(frameDeltaTimeSeconds);
            },
            [this]()
            {
                render();
            },
            deltaTimeSeconds);
    }

    return 0;
}

void Game::processEvents()
{
    while (const std::optional event = m_impl->window.pollEvent())
    {
        detail::RuntimeEvent runtimeEvent = detail::RuntimeEvent::None;

        if (event->is<sf::Event::Closed>())
        {
            runtimeEvent = detail::RuntimeEvent::WindowClosed;
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                runtimeEvent = detail::RuntimeEvent::EscapePressed;
            }
        }

        if (detail::shouldCloseForEvent(runtimeEvent))
        {
            m_impl->window.close();
        }
    }
}

void Game::update(float deltaTimeSeconds)
{
    m_impl->overworldRuntime.update(
        deltaTimeSeconds,
        {
            readMovementIntent(),
            {
                static_cast<float>(kWindowWidth),
                static_cast<float>(kWindowHeight)}}); 
}

void Game::render()
{
    m_impl->window.clear(kBackgroundColor);

    const OverworldFrameState& frameState = m_impl->overworldRuntime.getFrameState();
    sf::View view;
    view.setCenter({frameState.frame.center.x, frameState.frame.center.y});
    view.setSize({frameState.frame.size.width, frameState.frame.size.height});
    m_impl->window.setView(view);

    sf::RectangleShape tileShape;
    tileShape.setSize({frameState.tileSize, frameState.tileSize});

    for (const VisibleWorldTile& visibleTile : frameState.visibleTiles)
    {
        tileShape.setPosition({
            visibleTile.center.x - (frameState.tileSize * 0.5F),
            visibleTile.center.y - (frameState.tileSize * 0.5F)});
        tileShape.setFillColor(getTileColor(visibleTile.tileType));
        m_impl->window.draw(tileShape);
    }

    sf::RectangleShape playerMarker;
    playerMarker.setSize({
        frameState.playerMarker.size.width,
        frameState.playerMarker.size.height});
    playerMarker.setOrigin({
        frameState.playerMarker.origin.x,
        frameState.playerMarker.origin.y});
    playerMarker.setPosition({
        frameState.playerMarker.position.x,
        frameState.playerMarker.position.y});
    playerMarker.setFillColor(kPlayerColor);
    m_impl->window.draw(playerMarker);

    m_impl->window.display();
}

} // namespace rpg
