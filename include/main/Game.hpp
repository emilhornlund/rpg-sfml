/**
 * @file Game.hpp
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

#ifndef RPG_MAIN_GAME_HPP
#define RPG_MAIN_GAME_HPP

#include <memory>

/**
 * @brief Core runtime types for the RPG executable.
 */
namespace rpg
{

/**
 * @brief Owns the top-level game runtime and main loop.
 *
 * The Game class is the public entry point for the executable runtime. It is
 * responsible for creating the internal runtime implementation, driving the
 * application loop, and coordinating the high-level frame lifecycle.
 *
 * The public header intentionally hides SFML-specific types behind a private
 * implementation so this interface stays lightweight and stable while the
 * runtime internals evolve.
 */
class Game
{
public:
    /**
     * @brief Construct a new game runtime.
     *
     * Creates the internal runtime implementation and prepares the executable
     * for entering the main loop.
     */
    Game();

    /**
     * @brief Destroy the game runtime.
     *
     * Releases the internal runtime implementation and any resources owned
     * through that implementation boundary.
     */
    ~Game();

    /**
     * @brief Copy construction is disabled.
     *
     * The runtime owns unique state and therefore cannot be copied safely.
     */
    Game(const Game&) = delete;

    /**
     * @brief Copy assignment is disabled.
     *
     * The runtime owns unique state and therefore cannot be copied safely.
     *
     * @return This function is deleted and cannot be called.
     */
    Game& operator=(const Game&) = delete;

    /**
     * @brief Move-construct a game runtime.
     *
     * Transfers ownership of the internal runtime implementation from another
     * Game instance.
     *
     * @param other The source runtime to move from.
     */
    Game(Game&&) noexcept;

    /**
     * @brief Move-assign a game runtime.
     *
     * Transfers ownership of the internal runtime implementation from another
     * Game instance into this one.
     *
     * @param other The source runtime to move from.
     * @return A reference to this Game instance after the move completes.
     */
    Game& operator=(Game&&) noexcept;

    /**
     * @brief Run the main game loop.
     *
     * Enters the runtime loop and repeatedly processes input events, updates
     * simulation state, and renders the current frame until the window closes.
     *
     * @return Process exit code for the executable.
     */
    int run();

private:
    /**
     * @brief Private runtime implementation.
     *
     * This incomplete type stores framework-specific state and concrete runtime
     * collaborators so the public header does not depend on SFML headers.
     */
    class Impl;

    /**
     * @brief Poll and handle pending window and input events.
     */
    void processEvents();

    /**
     * @brief Advance runtime state for a single frame.
     *
     * @param deltaTimeSeconds Time elapsed since the previous frame in seconds.
     */
    void update(float deltaTimeSeconds);

    /**
     * @brief Render the current frame to the game window.
     *
     */
    void render();

    /**
     * @brief Opaque pointer to the concrete runtime implementation.
     */
    std::unique_ptr<Impl> m_impl;
};

} // namespace rpg

#endif // RPG_MAIN_GAME_HPP
