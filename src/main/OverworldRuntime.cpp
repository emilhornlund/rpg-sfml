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

namespace
{

[[nodiscard]] float getMovementSpeedScaleForDebugView(const OverworldInput::DebugViewState& debugViewState) noexcept
{
    constexpr float kDefaultDebugZoomPercent = 300.0F;

    if (!debugViewState.isEnabled || debugViewState.zoomPercent <= 0)
    {
        return 1.0F;
    }

    return kDefaultDebugZoomPercent / static_cast<float>(debugViewState.zoomPercent);
}

[[nodiscard]] constexpr OverworldRenderTile makeRenderTile(
    const VisibleWorldTile& visibleTile,
    const float tileSize) noexcept
{
    return {
        visibleTile.coordinates,
        visibleTile.tileType,
        {tileSize, tileSize},
        {tileSize * 0.5F, tileSize * 0.5F},
        visibleTile.center};
}

[[nodiscard]] OverworldRenderMarker makePlayerRenderMarker(
    const detail::PlayerSpritePlacement& placement,
    const Player& player) noexcept
{
    return {
        placement.size,
        placement.origin,
        placement.position,
        OverworldRenderMarkerAppearance::Player,
        player.getFacingDirection(),
        player.getWalkFrameIndex()};
}

[[nodiscard]] constexpr OverworldRenderContent makeRenderContent(const VisibleWorldContent& visibleContent) noexcept
{
    return {
        visibleContent.instance.id,
        visibleContent.instance.type,
        visibleContent.instance.footprint.size,
        {
            visibleContent.instance.footprint.size.width * 0.5F,
            visibleContent.instance.footprint.size.height * 0.5F,
        },
        visibleContent.instance.position,
        visibleContent.instance.appearanceId};
}

} // namespace

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
    refreshRenderSnapshot();
}

void OverworldRuntime::update(const float deltaTimeSeconds, const OverworldInput& input)
{
    applyDebugViewState(input.debugViewState);
    initialize(input.viewportSize);
    m_player.setMovementIntent(input.movementIntent);
    m_player.update(deltaTimeSeconds, m_world);
    m_camera.update(
        m_player.getPosition(),
        input.viewportSize.width,
        input.viewportSize.height);
    refreshRenderSnapshot();
}

void OverworldRuntime::applyDebugViewState(const OverworldInput::DebugViewState& debugViewState) noexcept
{
    m_camera.setZoomPercent(debugViewState.zoomPercent);
    m_player.setMovementSpeedScale(getMovementSpeedScaleForDebugView(debugViewState));
}

const OverworldRenderSnapshot& OverworldRuntime::getRenderSnapshot() const noexcept
{
    return m_renderSnapshot;
}

std::uint32_t OverworldRuntime::getWorldGenerationSeed() const noexcept
{
    return m_world.getGenerationSeed();
}

void OverworldRuntime::refreshRenderSnapshot()
{
    const float tileSize = m_world.getTileSize();
    m_renderSnapshot.cameraFrame = m_camera.getFrame();

    const std::vector<VisibleWorldTile> visibleTiles = m_world.getVisibleTiles(m_renderSnapshot.cameraFrame);
    m_renderSnapshot.visibleTiles.clear();
    m_renderSnapshot.visibleTiles.reserve(visibleTiles.size());

    for (const VisibleWorldTile& visibleTile : visibleTiles)
    {
        m_renderSnapshot.visibleTiles.push_back(makeRenderTile(visibleTile, tileSize));
    }

    const std::vector<VisibleWorldContent> visibleContent = m_world.getVisibleContent(m_renderSnapshot.cameraFrame);
    m_renderSnapshot.generatedContent.clear();
    m_renderSnapshot.generatedContent.reserve(visibleContent.size());

    for (const VisibleWorldContent& content : visibleContent)
    {
        m_renderSnapshot.generatedContent.push_back(makeRenderContent(content));
    }

    const detail::PlayerSpritePlacement placement = detail::getPlayerSpritePlacement(
        tileSize,
        m_player.getPosition());

    m_renderSnapshot.markers.clear();
    m_renderSnapshot.markers.push_back(makePlayerRenderMarker(placement, m_player));
}

} // namespace rpg
