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

#include "GameRuntimeSupport.hpp"

#include <algorithm>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <utility>

namespace rpg
{
constexpr unsigned int kWindowWidth = 1280;
constexpr unsigned int kWindowHeight = 720;
constexpr int kTerrainTilesetCellSize = 16;
constexpr int kPlayerSpritesheetCellSize = 48;
constexpr char kTerrainTilesetFilename[] = "overworld-terrain-tileset.png";
constexpr char kPlayerSpritesheetFilename[] = "player-walking-spritesheet.png";
const sf::Color kBackgroundColor(24, 24, 27);
const sf::Color kGridOverlayColor(255, 255, 255, 96);
constexpr float kGridOverlayThickness = 1.0F;

[[nodiscard]] std::optional<detail::OverworldDirectionalKey> getDirectionalKey(
    const sf::Keyboard::Key key) noexcept
{
    switch (key)
    {
    case sf::Keyboard::Key::A:
    case sf::Keyboard::Key::Left:
        return detail::OverworldDirectionalKey::Left;
    case sf::Keyboard::Key::D:
    case sf::Keyboard::Key::Right:
        return detail::OverworldDirectionalKey::Right;
    case sf::Keyboard::Key::W:
    case sf::Keyboard::Key::Up:
        return detail::OverworldDirectionalKey::Up;
    case sf::Keyboard::Key::S:
    case sf::Keyboard::Key::Down:
        return detail::OverworldDirectionalKey::Down;
    default:
        return std::nullopt;
    }
}

[[nodiscard]] std::optional<detail::OverworldDebugViewAction> getDebugViewAction(
    const sf::Keyboard::Key key) noexcept
{
    switch (key)
    {
    case sf::Keyboard::Key::Q:
        return detail::OverworldDebugViewAction::ZoomOut;
    case sf::Keyboard::Key::E:
        return detail::OverworldDebugViewAction::ZoomIn;
    case sf::Keyboard::Key::G:
        return detail::OverworldDebugViewAction::ToggleTileGrid;
    default:
        return std::nullopt;
    }
}

[[nodiscard]] std::filesystem::path getExecutableDirectory()
{
    return std::filesystem::read_symlink("/proc/self/exe").parent_path();
}

[[nodiscard]] std::filesystem::path getTerrainTilesetPath()
{
    return getExecutableDirectory() / "assets" / kTerrainTilesetFilename;
}

[[nodiscard]] std::filesystem::path getPlayerSpritesheetPath()
{
    return getExecutableDirectory() / "assets" / kPlayerSpritesheetFilename;
}

[[nodiscard]] sf::Texture loadTerrainTileset()
{
    sf::Texture terrainTileset;
    const std::filesystem::path terrainTilesetPath = getTerrainTilesetPath();

    if (!terrainTileset.loadFromFile(terrainTilesetPath.string()))
    {
        throw std::runtime_error("Failed to load overworld terrain tileset from " + terrainTilesetPath.string());
    }

    terrainTileset.setSmooth(false);
    return terrainTileset;
}

[[nodiscard]] sf::Texture loadPlayerSpritesheet()
{
    sf::Texture playerSpritesheet;
    const std::filesystem::path playerSpritesheetPath = getPlayerSpritesheetPath();

    if (!playerSpritesheet.loadFromFile(playerSpritesheetPath.string()))
    {
        throw std::runtime_error("Failed to load player spritesheet from " + playerSpritesheetPath.string());
    }

    playerSpritesheet.setSmooth(false);
    return playerSpritesheet;
}

[[nodiscard]] sf::IntRect getTerrainTilesetRect(const TileType tileType) noexcept
{
    switch (tileType)
    {
    case TileType::Grass:
        return {{0, 0}, {kTerrainTilesetCellSize, kTerrainTilesetCellSize}};
    case TileType::Sand:
        return {{kTerrainTilesetCellSize, 0}, {kTerrainTilesetCellSize, kTerrainTilesetCellSize}};
    case TileType::Water:
        return {{0, kTerrainTilesetCellSize}, {kTerrainTilesetCellSize, kTerrainTilesetCellSize}};
    case TileType::Forest:
        return {{kTerrainTilesetCellSize, kTerrainTilesetCellSize}, {kTerrainTilesetCellSize, kTerrainTilesetCellSize}};
    }

    return {{0, 0}, {kTerrainTilesetCellSize, kTerrainTilesetCellSize}};
}

[[nodiscard]] int getPlayerSpritesheetRow(const PlayerFacingDirection facingDirection) noexcept
{
    switch (facingDirection)
    {
    case PlayerFacingDirection::Down:
        return 0;
    case PlayerFacingDirection::Left:
        return 1;
    case PlayerFacingDirection::Right:
        return 2;
    case PlayerFacingDirection::Up:
        return 3;
    }

    return 0;
}

[[nodiscard]] sf::IntRect getPlayerSpritesheetRect(const OverworldRenderMarker& renderMarker) noexcept
{
    const int frameIndex = std::clamp(renderMarker.animationFrameIndex, 0, 2);
    const int rowIndex = getPlayerSpritesheetRow(renderMarker.facingDirection);
    return {
        {frameIndex * kPlayerSpritesheetCellSize, rowIndex * kPlayerSpritesheetCellSize},
        {kPlayerSpritesheetCellSize, kPlayerSpritesheetCellSize}};
}

class Game::Impl
{
public:
    Impl()
        : window(sf::VideoMode({kWindowWidth, kWindowHeight}), "rpg-sfml", sf::State::Windowed)
        , terrainTileset(loadTerrainTileset())
        , playerSpritesheet(loadPlayerSpritesheet())
    {
        window.setFramerateLimit(60);
    }

    sf::RenderWindow window;
    sf::Texture terrainTileset;
    sf::Texture playerSpritesheet;
    OverworldRuntime overworldRuntime;
    detail::OverworldDirectionalInput directionalInput;
    OverworldInput::DebugViewState debugViewState = detail::makeOverworldDebugViewState(detail::isDebugViewModeEnabledForBuild());
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
    while (const std::optional event = m_impl->window.pollEvent())
    {
        detail::RuntimeEvent runtimeEvent = detail::RuntimeEvent::None;

        if (event->is<sf::Event::Closed>())
        {
            runtimeEvent = detail::RuntimeEvent::WindowClosed;
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (const std::optional directionalKey = getDirectionalKey(keyPressed->code);
                directionalKey.has_value())
            {
                detail::applyDirectionalInputPress(m_impl->directionalInput, *directionalKey);
            }

            if (const std::optional debugViewAction = getDebugViewAction(keyPressed->code);
                debugViewAction.has_value())
            {
                detail::applyOverworldDebugViewAction(m_impl->debugViewState, *debugViewAction);
            }

            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                runtimeEvent = detail::RuntimeEvent::EscapePressed;
            }
        }
        else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            if (const std::optional directionalKey = getDirectionalKey(keyReleased->code);
                directionalKey.has_value())
            {
                detail::applyDirectionalInputRelease(m_impl->directionalInput, *directionalKey);
            }
        }

        if (detail::shouldCloseForEvent(runtimeEvent))
        {
            m_impl->window.close();
        }
    }
}

void Game::update(float deltaTimeSeconds)
{
    const WorldSize viewportSize{
        static_cast<float>(kWindowWidth),
        static_cast<float>(kWindowHeight)};

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
    sf::View view;
    view.setCenter({renderSnapshot.cameraFrame.center.x, renderSnapshot.cameraFrame.center.y});
    view.setSize({renderSnapshot.cameraFrame.size.width, renderSnapshot.cameraFrame.size.height});
    m_impl->window.setView(view);

    sf::Sprite tileSprite(m_impl->terrainTileset);

    for (const OverworldRenderTile& visibleTile : renderSnapshot.visibleTiles)
    {
        const float scaleX = visibleTile.size.width / static_cast<float>(kTerrainTilesetCellSize);
        const float scaleY = visibleTile.size.height / static_cast<float>(kTerrainTilesetCellSize);
        tileSprite.setTextureRect(getTerrainTilesetRect(visibleTile.tileType));
        tileSprite.setScale({scaleX, scaleY});
        tileSprite.setOrigin({visibleTile.origin.x / scaleX, visibleTile.origin.y / scaleY});
        tileSprite.setPosition({visibleTile.position.x, visibleTile.position.y});
        m_impl->window.draw(tileSprite);
    }

    if (detail::shouldRenderTileGridOverlay(m_impl->debugViewState))
    {
        sf::RectangleShape gridSegment;
        gridSegment.setFillColor(kGridOverlayColor);

        for (const OverworldRenderTile& visibleTile : renderSnapshot.visibleTiles)
        {
            const auto overlayRectangles = detail::getTileGridOverlayRectangles(visibleTile, kGridOverlayThickness);

            for (const detail::OverlayRectangle& overlayRectangle : overlayRectangles)
            {
                gridSegment.setSize({overlayRectangle.size.width, overlayRectangle.size.height});
                gridSegment.setPosition({overlayRectangle.position.x, overlayRectangle.position.y});
                m_impl->window.draw(gridSegment);
            }
        }
    }

    sf::Sprite playerSprite(m_impl->playerSpritesheet);

    for (const OverworldRenderMarker& renderMarker : renderSnapshot.markers)
    {
        const float scaleX = renderMarker.size.width / static_cast<float>(kPlayerSpritesheetCellSize);
        const float scaleY = renderMarker.size.height / static_cast<float>(kPlayerSpritesheetCellSize);
        playerSprite.setTextureRect(getPlayerSpritesheetRect(renderMarker));
        playerSprite.setScale({scaleX, scaleY});
        playerSprite.setOrigin({renderMarker.origin.x / scaleX, renderMarker.origin.y / scaleY});
        playerSprite.setPosition({renderMarker.position.x, renderMarker.position.y});
        m_impl->window.draw(playerSprite);
    }

    m_impl->window.display();
}

} // namespace rpg
