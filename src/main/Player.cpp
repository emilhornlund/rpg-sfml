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
constexpr float kWalkFrameDurationSeconds = 0.12F;
constexpr int kWalkAnimationFrames[kWalkAnimationStepCount] = {1, 2, 1, 0};

[[nodiscard]] float vectorLength(const MovementIntent& movementIntent) noexcept
{
    return std::sqrt(
        movementIntent.x * movementIntent.x
        + movementIntent.y * movementIntent.y);
}

[[nodiscard]] MovementIntent normalize(const MovementIntent& movementIntent) noexcept
{
    const float length = vectorLength(movementIntent);

    if (length <= 0.0F)
    {
        return {0.0F, 0.0F};
    }

    return {movementIntent.x / length, movementIntent.y / length};
}

[[nodiscard]] PlayerFacingDirection resolveFacingDirection(
    const MovementIntent& movementIntent,
    const PlayerFacingDirection currentFacingDirection) noexcept
{
    const float absoluteX = std::fabs(movementIntent.x);
    const float absoluteY = std::fabs(movementIntent.y);

    if (absoluteX <= 0.0F && absoluteY <= 0.0F)
    {
        return currentFacingDirection;
    }

    if (absoluteY >= absoluteX)
    {
        return movementIntent.y < 0.0F
            ? PlayerFacingDirection::Up
            : PlayerFacingDirection::Down;
    }

    return movementIntent.x < 0.0F
        ? PlayerFacingDirection::Left
        : PlayerFacingDirection::Right;
}

void applyAxisConstrainedStep(WorldPosition& position, const WorldPosition& step, const World& world) noexcept
{
    const WorldPosition candidate{position.x + step.x, position.y + step.y};

    if (world.isTraversable(candidate))
    {
        position = candidate;
        return;
    }

    const WorldPosition xOnlyCandidate{position.x + step.x, position.y};

    if (world.isTraversable(xOnlyCandidate))
    {
        position = xOnlyCandidate;
    }

    const WorldPosition yOnlyCandidate{position.x, position.y + step.y};

    if (world.isTraversable(yOnlyCandidate))
    {
        position = yOnlyCandidate;
    }
}

} // namespace

Player::Player() = default;

Player::~Player() = default;

void Player::spawn(const WorldPosition& position) noexcept
{
    m_state.position = position;
    m_state.movementIntent = {0.0F, 0.0F};
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

void Player::update(const float deltaTimeSeconds, const World& world) noexcept
{
    if (!m_state.isSpawned)
    {
        return;
    }

    const float clampedDeltaTimeSeconds = std::max(deltaTimeSeconds, 0.0F);
    m_state.facingDirection = resolveFacingDirection(m_state.movementIntent, m_state.facingDirection);
    const MovementIntent direction = normalize(m_state.movementIntent);

    if (direction.x == 0.0F && direction.y == 0.0F)
    {
        m_state.animationElapsedSeconds = 0.0F;
        m_state.walkAnimationStepIndex = 0;
        m_state.isMoving = false;
        return;
    }

    m_state.isMoving = true;

    const float travelDistance = m_state.movementSpeed * clampedDeltaTimeSeconds;

    if (travelDistance <= 0.0F)
    {
        return;
    }

    const float maxStepDistance = std::max(world.getTileSize() * 0.25F, 1.0F);
    const int steps = std::max(1, static_cast<int>(std::ceil(travelDistance / maxStepDistance)));
    const WorldPosition step{
        direction.x * (travelDistance / static_cast<float>(steps)),
        direction.y * (travelDistance / static_cast<float>(steps))};

    for (int stepIndex = 0; stepIndex < steps; ++stepIndex)
    {
        applyAxisConstrainedStep(m_state.position, step, world);
    }

    m_state.animationElapsedSeconds += clampedDeltaTimeSeconds;

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
    return m_state.movementSpeed;
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
