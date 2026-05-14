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
#include <main/OverworldRuntime.hpp>

#include "GameDebugOverlaySupport.hpp"
#include "GameEventSupport.hpp"
#include "GameOcclusionCompositeSupport.hpp"
#include "GameRenderPlanSupport.hpp"
#include "GameRenderSupport.hpp"
#include "GameSceneRenderSupport.hpp"
#include "GameResourceBootstrapSupport.hpp"
#include "GameRuntimeSupport.hpp"

#include <algorithm>
#include <limits>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rpg
{
constexpr unsigned int kWindowWidth = 1280;
constexpr unsigned int kWindowHeight = 720;
const sf::Color kBackgroundColor(24, 24, 27);
const sf::Color kGridOverlayColor(255, 255, 255, 96);
constexpr float kGridOverlayThickness = 1.0F;
class Game::Impl
{
public:
    Impl()
        : window(sf::VideoMode({kWindowWidth, kWindowHeight}), "rpg-sfml", sf::State::Windowed)
        , renderResources(detail::loadGameRenderResources())
        , debugOverlay(detail::makeDebugOverlayRuntime())
    {
        const detail::WindowFramePacingConfig framePacing = detail::getDefaultWindowFramePacingConfig();
        window.setVerticalSyncEnabled(framePacing.mode == detail::WindowFramePacingMode::VerticalSync);
        window.setFramerateLimit(
            framePacing.mode == detail::WindowFramePacingMode::FramerateLimit ? framePacing.framerateLimit : 0U);
        detail::updateScreenSpaceOverlayView(overlayView, window.getSize());
    }

    sf::RenderWindow window;
    sf::View overlayView;
    detail::GameRenderResources renderResources;
    detail::PlayerOcclusionCompositeRuntime playerOcclusionComposite;
    detail::DebugOverlayRuntime debugOverlay;
    OverworldRuntime overworldRuntime;
    detail::OverworldDirectionalInput directionalInput;
    OverworldInput::DebugViewState debugViewState = detail::makeOverworldDebugViewState(detail::isDebugViewModeEnabledForBuild());
    float terrainAnimationElapsedSeconds = 0.0F;
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
    if (detail::processGameEvents(
            m_impl->window,
            {
                m_impl->overlayView,
                m_impl->directionalInput,
                m_impl->debugViewState,
                m_impl->debugOverlay,
            }))
    {
        m_impl->window.close();
    }
}

void Game::update(float deltaTimeSeconds)
{
    const sf::Vector2u windowSize = m_impl->window.getSize();
    const WorldSize viewportSize = detail::makeViewportSize(windowSize.x, windowSize.y);
    detail::updateDebugOverlayRuntime(m_impl->debugOverlay, deltaTimeSeconds);
    m_impl->terrainAnimationElapsedSeconds += deltaTimeSeconds;

    m_impl->overworldRuntime.update(
        deltaTimeSeconds,
        detail::getOverworldInput(
            m_impl->directionalInput,
            viewportSize,
            m_impl->debugViewState));
}

void Game::render()
{
    m_impl->window.clear(kBackgroundColor);

    const OverworldRenderSnapshot& renderSnapshot = m_impl->overworldRuntime.getRenderSnapshot();
    const OverworldDebugSnapshot& debugSnapshot = m_impl->overworldRuntime.getDebugSnapshot();
    sf::View view;
    detail::applyViewFrame(view, renderSnapshot.cameraFrame);

    const std::uint32_t worldGenerationSeed = m_impl->overworldRuntime.getWorldGenerationSeed();
    detail::OverworldSceneRenderFrame renderFrame = detail::buildOverworldSceneRenderFrame(
        m_impl->renderResources,
        renderSnapshot,
        m_impl->terrainAnimationElapsedSeconds,
        worldGenerationSeed,
        detail::shouldRenderTileGridOverlay(m_impl->debugViewState),
        kGridOverlayThickness,
        kGridOverlayColor);

    const detail::OverworldRenderPlan renderPlan = detail::buildOverworldRenderPlan(
        renderSnapshot,
        renderFrame.terrainVertexArray.getVertexCount(),
        renderFrame.tileGridVertexArray.getVertexCount());
    detail::renderOverworldScene(m_impl->window, view, renderFrame, renderSnapshot, renderPlan);

    if (renderPlan.playerMarkerIndex.has_value() && !renderPlan.overlapQualifiedOcclusionCandidateIndices.empty())
    {
        const OverworldRenderMarker& playerMarker = renderSnapshot.markers[*renderPlan.playerMarkerIndex];
        detail::drawPlayerOcclusionComposite(
            m_impl->window,
            m_impl->playerOcclusionComposite,
            m_impl->renderResources.playerOcclusionShader,
            view,
            m_impl->overlayView,
            renderFrame.playerSprite,
            renderFrame.vegetationSprite,
            m_impl->renderResources.vegetationTilesetMetadata,
            renderSnapshot.generatedContent,
            playerMarker,
            renderPlan.overlapQualifiedOcclusionCandidateIndices,
            renderFrame.worldTileSize);
        m_impl->window.setView(view);
    }

    detail::drawDebugOverlay(
        m_impl->window,
        m_impl->overlayView,
        m_impl->debugOverlay,
        debugSnapshot,
        renderPlan.debugOverlayRenderMetrics);

    m_impl->window.display();
}

} // namespace rpg
