/**
 * @file Game.hpp
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

#ifndef RPG_SFML_GAME_HPP
#define RPG_SFML_GAME_HPP

#include <memory>

/**
 * Forward declaration to avoid pulling heavy SFML headers into the public API.
 */
namespace sf
{
class RenderWindow;
}

namespace rpg
{

/**
 * @brief Owns the main loop and render window for the RPG application.
 */
class Game final
{
public:
    /**
     * @brief Construct a non-running game instance.
     *
     * The window is not created until run() is called.
     */
    Game();

    /**
     * @brief Destroy the game instance and release owned resources.
     */
    ~Game();

    /**
     * @brief Deleted copy constructor
     */
    Game(const Game&) = delete;

    /**
     * @brief Deleted copy assignment
     *
     * @return description here.
     */
    Game& operator=(const Game&) = delete;

    /**
     * @brief Enter the main loop.
     *
     * Creates the render window, processes events, and repeatedly calls
     * update() and draw() until quit() is invoked.
     *
     * Calling run() while the game is already running is a no-op and returns
     * the last exit code.
     *
     * @return Process exit code provided via quit() (defaults to 0).
     */
    [[nodiscard]] int run();


    /**
     * @brief Request termination of the main loop.
     *
     * Safe to call from inside the event loop. The window is closed and the
     * next iteration will exit the loop, returning the specified code.
     *
     * @param exitCode Exit code to return from run().
     */
    void quit(int exitCode) noexcept;

private:
    /**
     * @brief Advance simulation state.
     *
     * Called once per frame by run().
     *
     * @param deltaTime Seconds elapsed since the previous frame.
     */
    void update([[maybe_unused]] float deltaTime) noexcept;

    /**
     * @brief Render the current frame.
     *
     * Called once per frame by run() after update().
     */
    void draw() noexcept;

    int                               m_exitCode{0};    //!< Exit code returned from run().
    bool                              m_running{false}; //!< True while the main loop is active.
    std::unique_ptr<sf::RenderWindow> m_renderWindow;   //!< Owned SFML render window.

}; // class Game

} // namespace rpg

#endif //RPG_SFML_GAME_HPP
