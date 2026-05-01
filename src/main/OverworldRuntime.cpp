/**
 * @file OverworldRuntime.cpp
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

#include <main/OverworldRuntime.hpp>

#include "GameRuntimeSupport.hpp"

namespace rpg
{

OverworldRuntime::OverworldRuntime() = default;

OverworldRuntime::~OverworldRuntime() = default;

void OverworldRuntime::initialize(const WorldSize& viewportSize)
{
    if (!m_isInitialized)
    {
        m_player.spawn(m_world.getSpawnPosition());
        m_isInitialized = true;
    }

    m_camera.update(
        m_player.getPosition(),
        viewportSize.width,
        viewportSize.height);
    refreshFrameState();
}

void OverworldRuntime::update(const float deltaTimeSeconds, const OverworldInput& input)
{
    initialize(input.viewportSize);
    m_player.setMovementIntent(input.movementIntent);
    m_player.update(deltaTimeSeconds, m_world);
    m_camera.update(
        m_player.getPosition(),
        input.viewportSize.width,
        input.viewportSize.height);
    refreshFrameState();
}

const OverworldFrameState& OverworldRuntime::getFrameState() const noexcept
{
    return m_frameState;
}

void OverworldRuntime::refreshFrameState()
{
    m_frameState.tileSize = m_world.getTileSize();
    m_frameState.frame = m_camera.getFrame();
    m_frameState.visibleTiles = m_world.getVisibleTiles(m_frameState.frame);

    const detail::PlayerMarkerPlacement placement = detail::getPlayerMarkerPlacement(
        m_frameState.tileSize,
        m_player.getPosition());
    m_frameState.playerMarker = {
        placement.size,
        placement.origin,
        placement.position};
}

} // namespace rpg
