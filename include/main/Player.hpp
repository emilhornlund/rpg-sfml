/**
 * @file Player.hpp
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

#ifndef RPG_MAIN_PLAYER_HPP
#define RPG_MAIN_PLAYER_HPP

/**
 * @brief Core runtime types for the RPG executable.
 */
namespace rpg
{

/**
 * @brief Owns player-specific runtime state.
 *
 * The Player class provides the dedicated boundary for player-facing gameplay
 * data and behavior. It gives the runtime a stable type where future movement,
 * stats, input-driven actions, and other player rules can live without moving
 * them into the top-level coordinator.
 */
class Player
{
public:
    /**
     * @brief Construct a new player module.
     *
     * Prepares the player-owned state used by the runtime.
     */
    Player();

    /**
     * @brief Destroy the player module.
     *
     * Releases any resources retained by the player-owned state.
     */
    ~Player();

private:
    /**
     * @brief Internal player state container.
     *
     * The nested state keeps the public type ready for future internal
     * expansion without exposing implementation details in the header.
     */
    struct State
    {
    };

    /**
     * @brief Storage for player-owned runtime state.
     */
    State m_state;
};

} // namespace rpg

#endif // RPG_MAIN_PLAYER_HPP
