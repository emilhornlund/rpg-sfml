/**
 * @file Player.cpp
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

#include <main/Player.hpp>
#include <main/World.hpp>

#include <algorithm>
#include <cmath>

namespace rpg
{

namespace
{

constexpr int kWalkAnimationStepCount = 4;
constexpr float kWalkFrameDurationSeconds = 0.16F;
constexpr int kWalkAnimationFrames[kWalkAnimationStepCount] = {1, 2, 1, 0};
constexpr float kMovementEpsilon = 0.001F;

[[nodiscard]] bool isMovementIntentZero(const MovementIntent& movementIntent) noexcept
{
    return std::fabs(movementIntent.x) <= kMovementEpsilon
        && std::fabs(movementIntent.y) <= kMovementEpsilon;
}

[[nodiscard]] MovementIntent getCardinalMovementIntent(const MovementIntent& movementIntent) noexcept
{
    if (isMovementIntentZero(movementIntent))
    {
        return {0.0F, 0.0F};
    }

    const float absoluteX = std::fabs(movementIntent.x);
    const float absoluteY = std::fabs(movementIntent.y);

    if (absoluteX >= absoluteY)
    {
        return {movementIntent.x < 0.0F ? -1.0F : 1.0F, 0.0F};
    }

    return {0.0F, movementIntent.y < 0.0F ? -1.0F : 1.0F};
}

[[nodiscard]] PlayerFacingDirection resolveFacingDirection(
    const MovementIntent& movementIntent,
    const PlayerFacingDirection currentFacingDirection) noexcept
{
    if (isMovementIntentZero(movementIntent))
    {
        return currentFacingDirection;
    }

    if (std::fabs(movementIntent.x) > std::fabs(movementIntent.y))
    {
        return movementIntent.x < 0.0F
            ? PlayerFacingDirection::Left
            : PlayerFacingDirection::Right;
    }

    return movementIntent.y < 0.0F
        ? PlayerFacingDirection::Up
        : PlayerFacingDirection::Down;
}

[[nodiscard]] float getDistance(
    const WorldPosition& from,
    const WorldPosition& to) noexcept
{
    const float deltaX = to.x - from.x;
    const float deltaY = to.y - from.y;
    return std::sqrt(deltaX * deltaX + deltaY * deltaY);
}

} // namespace

Player::Player() = default;

Player::~Player() = default;

void Player::spawn(const WorldPosition& position) noexcept
{
    m_state.position = position;
    m_state.stepDestination = position;
    m_state.movementIntent = {0.0F, 0.0F};
    m_state.activeStepDirection = {0.0F, 0.0F};
    m_state.animationElapsedSeconds = 0.0F;
    m_state.walkAnimationStepIndex = 0;
    m_state.facingDirection = PlayerFacingDirection::Down;
    m_state.isMoving = false;
    m_state.isSpawned = true;
}

void Player::setMovementIntent(const MovementIntent& movementIntent) noexcept
{
    m_state.movementIntent = movementIntent;
}

void Player::setMovementSpeedScale(const float movementSpeedScale) noexcept
{
    m_state.movementSpeedScale = std::max(movementSpeedScale, 0.0F);
}

void Player::update(const float deltaTimeSeconds, const World& world) noexcept
{
    if (!m_state.isSpawned)
    {
        return;
    }

    const float clampedDeltaTimeSeconds = std::max(deltaTimeSeconds, 0.0F);
    const float movementSpeed = std::max(getMovementSpeed(), 0.0F);
    const MovementIntent requestedDirection = getCardinalMovementIntent(m_state.movementIntent);
    float remainingDistance = movementSpeed * clampedDeltaTimeSeconds;
    float movedDistance = 0.0F;

    if (!m_state.isMoving && !isMovementIntentZero(requestedDirection))
    {
        m_state.facingDirection = resolveFacingDirection(requestedDirection, m_state.facingDirection);
    }

    while (remainingDistance > 0.0F)
    {
        if (!m_state.isMoving)
        {
            if (isMovementIntentZero(requestedDirection))
            {
                break;
            }

            const TileCoordinates currentTile = world.getTileCoordinates(m_state.position);
            m_state.position = world.getTileCenter(currentTile);

            const TileCoordinates destinationTile{
                currentTile.x + static_cast<int>(requestedDirection.x),
                currentTile.y + static_cast<int>(requestedDirection.y)};

            if (!world.isTraversable(destinationTile))
            {
                break;
            }

            m_state.stepDestination = world.getTileCenter(destinationTile);
            m_state.activeStepDirection = requestedDirection;
            m_state.facingDirection = resolveFacingDirection(m_state.activeStepDirection, m_state.facingDirection);
            m_state.isMoving = true;
        }

        const float distanceToDestination = getDistance(m_state.position, m_state.stepDestination);

        if (distanceToDestination <= kMovementEpsilon)
        {
            m_state.position = m_state.stepDestination;
            m_state.isMoving = false;
            continue;
        }

        const float distanceAdvanced = std::min(remainingDistance, distanceToDestination);
        m_state.position.x += m_state.activeStepDirection.x * distanceAdvanced;
        m_state.position.y += m_state.activeStepDirection.y * distanceAdvanced;
        movedDistance += distanceAdvanced;
        remainingDistance -= distanceAdvanced;

        if (distanceAdvanced + kMovementEpsilon >= distanceToDestination)
        {
            m_state.position = m_state.stepDestination;
            m_state.isMoving = false;
        }
        else
        {
            break;
        }
    }

    if (!m_state.isMoving)
    {
        m_state.animationElapsedSeconds = 0.0F;
        m_state.walkAnimationStepIndex = 0;
        return;
    }

    if (movementSpeed <= 0.0F || movedDistance <= 0.0F)
    {
        return;
    }

    m_state.animationElapsedSeconds += movedDistance / movementSpeed;

    while (m_state.animationElapsedSeconds >= kWalkFrameDurationSeconds)
    {
        m_state.animationElapsedSeconds -= kWalkFrameDurationSeconds;
        m_state.walkAnimationStepIndex = (m_state.walkAnimationStepIndex + 1) % kWalkAnimationStepCount;
    }
}

WorldPosition Player::getPosition() const noexcept
{
    return m_state.position;
}

float Player::getMovementSpeed() const noexcept
{
    return m_state.baseMovementSpeed * m_state.movementSpeedScale;
}

float Player::getMovementSpeedScale() const noexcept
{
    return m_state.movementSpeedScale;
}

PlayerFacingDirection Player::getFacingDirection() const noexcept
{
    return m_state.facingDirection;
}

bool Player::isMoving() const noexcept
{
    return m_state.isMoving;
}

int Player::getWalkFrameIndex() const noexcept
{
    return kWalkAnimationFrames[m_state.walkAnimationStepIndex];
}

} // namespace rpg
