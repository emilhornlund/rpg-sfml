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

#include <main/OverworldTypes.hpp>

/**
 * @brief Core runtime types for the RPG executable.
 */
namespace rpg
{

class World;

/**
 * @brief Cardinal directions used for player presentation.
 */
enum class PlayerFacingDirection
{
    Down,
    Left,
    Right,
    Up
};

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

    /**
     * @brief Place the player at a world-space position.
     *
     * @param position Position to use as the player's starting point.
     */
    void spawn(const WorldPosition& position) noexcept;

    /**
     * @brief Set the current movement intent.
     *
     * @param movementIntent Desired movement direction for the next update.
     */
    void setMovementIntent(const MovementIntent& movementIntent) noexcept;

    /**
     * @brief Advance the player simulation.
     *
     * @param deltaTimeSeconds Frame delta time in seconds.
     * @param world World data used for traversability checks.
     */
    void update(float deltaTimeSeconds, const World& world) noexcept;

    /**
     * @brief Read the player's current position.
     *
     * @return Current player position in world-space units.
     */
    [[nodiscard]] WorldPosition getPosition() const noexcept;

    /**
     * @brief Read the player's movement speed.
     *
     * @return Movement speed in world-space units per second.
     */
    [[nodiscard]] float getMovementSpeed() const noexcept;

    /**
     * @brief Read the player's current facing direction.
     *
     * @return Current presentation-facing direction.
     */
    [[nodiscard]] PlayerFacingDirection getFacingDirection() const noexcept;

    /**
     * @brief Read whether the player is currently moving.
     *
     * @return True when the last update advanced player movement.
     */
    [[nodiscard]] bool isMoving() const noexcept;

    /**
     * @brief Read the current walk animation frame index.
     *
     * @return Active frame index in the 3-frame walking row.
     */
    [[nodiscard]] int getWalkFrameIndex() const noexcept;

private:
    /**
     * @brief Internal player state container.
     *
     * The nested state keeps the public type ready for future internal
     * expansion without exposing implementation details in the header.
     */
    struct State
    {
        WorldPosition position{0.0F, 0.0F};
        WorldPosition stepDestination{0.0F, 0.0F};
        MovementIntent movementIntent{0.0F, 0.0F};
        MovementIntent activeStepDirection{0.0F, 0.0F};
        float movementSpeed = 160.0F;
        float animationElapsedSeconds = 0.0F;
        int walkAnimationStepIndex = 0;
        PlayerFacingDirection facingDirection = PlayerFacingDirection::Down;
        bool isMoving = false;
        bool isSpawned = false;
    };

    /**
     * @brief Storage for player-owned runtime state.
     */
    State m_state;
};

} // namespace rpg

#endif // RPG_MAIN_PLAYER_HPP
