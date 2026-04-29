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
#include <main/Camera.hpp>
#include <main/Player.hpp>
#include <main/World.hpp>

#include "GameRuntimeSupport.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>

#include <optional>
#include <utility>

namespace rpg
{
constexpr unsigned int kWindowWidth = 1280;
constexpr unsigned int kWindowHeight = 720;
const sf::Color kBackgroundColor(24, 24, 27);

class Game::Impl
{
public:
    Impl()
        : window(sf::VideoMode({kWindowWidth, kWindowHeight}), "rpg-sfml", sf::State::Windowed)
    {
        window.setFramerateLimit(60);
    }

    sf::RenderWindow window;
    World world;
    Player player;
    Camera camera;
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
    (void)deltaTimeSeconds;
    (void)m_impl->world;
    (void)m_impl->player;
    (void)m_impl->camera;
}

void Game::render()
{
    m_impl->window.clear(kBackgroundColor);
    m_impl->window.display();
}

} // namespace rpg
