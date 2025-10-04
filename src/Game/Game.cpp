/**
 * @file Game.cpp
 *
 * MIT License
 *
 * Copyright (c) 2025 Emil Hörnlund
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

#include <RPG/Game/Game.hpp>

#include <SFML/Graphics.hpp>

rpg::Game::Game() : m_renderWindow(nullptr)
{
}

rpg::Game::~Game() = default;

int rpg::Game::run()
{
    if (this->m_running)
    {
        return this->m_exitCode;
    }

    this->m_renderWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode({800u, 600u}), "RPG SFML");

    this->m_renderWindow->setVerticalSyncEnabled(true);

    this->m_running = true;

    sf::Clock clock;
    while (this->m_running)
    {
        while (const auto event = this->m_renderWindow->pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                this->quit(0);
            }
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                if (key->scancode == sf::Keyboard::Scan::Escape)
                {
                    this->quit(0);
                }
            }
        }

        const float dt = clock.restart().asSeconds();
        this->update(dt);

        this->m_renderWindow->clear(sf::Color::Black);
        this->draw();
        this->m_renderWindow->display();
    }

    return this->m_exitCode;
}

void rpg::Game::quit(const int exitCode) noexcept
{
    this->m_exitCode = exitCode;
    this->m_running  = false;

    if (this->m_renderWindow)
    {
        this->m_renderWindow->close();
    }
}

void rpg::Game::update([[maybe_unused]] float deltaTime) noexcept
{
}

void rpg::Game::draw() noexcept
{
}
