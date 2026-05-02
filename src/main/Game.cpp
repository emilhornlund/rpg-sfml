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
#include "TerrainAutotileSupport.hpp"

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
#include <cstdint>
#include <map>
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
constexpr char kTerrainTilesetClassificationFilename[] = "overworld-terrain-tileset-classification.json";
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

[[nodiscard]] std::filesystem::path getTerrainTilesetClassificationPath()
{
    return getExecutableDirectory() / "assets" / kTerrainTilesetClassificationFilename;
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

[[nodiscard]] detail::TerrainTilesetMetadata loadTerrainTilesetMetadata()
{
    return detail::TerrainTilesetMetadata::loadFromFile(getTerrainTilesetClassificationPath());
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

[[nodiscard]] sf::IntRect getTerrainTilesetRect(const detail::TerrainAtlasCell& cell) noexcept
{
    return {
        {cell.tileX * kTerrainTilesetCellSize, cell.tileY * kTerrainTilesetCellSize},
        {kTerrainTilesetCellSize, kTerrainTilesetCellSize}};
}

using VisibleTileTypeMap = std::map<std::pair<int, int>, TileType>;

[[nodiscard]] VisibleTileTypeMap buildVisibleTileTypeMap(const OverworldRenderSnapshot& renderSnapshot)
{
    VisibleTileTypeMap visibleTileTypes;

    for (const OverworldRenderTile& visibleTile : renderSnapshot.visibleTiles)
    {
        visibleTileTypes[{visibleTile.coordinates.x, visibleTile.coordinates.y}] = visibleTile.tileType;
    }

    return visibleTileTypes;
}

[[nodiscard]] std::array<TileType, 8> getNeighborTileTypes(
    const VisibleTileTypeMap& visibleTileTypes,
    const OverworldRenderTile& visibleTile) noexcept
{
    const auto getTileTypeOrCurrent = [&visibleTileTypes, &visibleTile](const int x, const int y) noexcept
    {
        const auto tileIt = visibleTileTypes.find({x, y});
        return tileIt == visibleTileTypes.end() ? visibleTile.tileType : tileIt->second;
    };

    return {
        getTileTypeOrCurrent(visibleTile.coordinates.x, visibleTile.coordinates.y - 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x + 1, visibleTile.coordinates.y - 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x + 1, visibleTile.coordinates.y),
        getTileTypeOrCurrent(visibleTile.coordinates.x + 1, visibleTile.coordinates.y + 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x, visibleTile.coordinates.y + 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x - 1, visibleTile.coordinates.y + 1),
        getTileTypeOrCurrent(visibleTile.coordinates.x - 1, visibleTile.coordinates.y),
        getTileTypeOrCurrent(visibleTile.coordinates.x - 1, visibleTile.coordinates.y - 1)};
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

[[nodiscard]] sf::Color getGeneratedContentBaseColor(const ContentType type) noexcept
{
    switch (type)
    {
    case ContentType::SpawnSite:
        return {98, 190, 122, 224};
    case ContentType::PointOfInterest:
        return {214, 163, 74, 224};
    }

    return {180, 180, 180, 224};
}

[[nodiscard]] sf::Color getGeneratedContentColor(const OverworldRenderContent& renderContent) noexcept
{
    const sf::Color baseColor = getGeneratedContentBaseColor(renderContent.type);
    const int variation = static_cast<int>(renderContent.appearanceId.value % 41U) - 20;
    const auto clampChannel = [variation](const int channel)
    {
        return static_cast<std::uint8_t>(std::clamp(channel + variation, 0, 255));
    };

    return {
        clampChannel(baseColor.r),
        clampChannel(baseColor.g),
        clampChannel(baseColor.b),
        baseColor.a};
}

class Game::Impl
{
public:
    Impl()
        : window(sf::VideoMode({kWindowWidth, kWindowHeight}), "rpg-sfml", sf::State::Windowed)
        , terrainTileset(loadTerrainTileset())
        , terrainTilesetMetadata(loadTerrainTilesetMetadata())
        , playerSpritesheet(loadPlayerSpritesheet())
    {
        window.setFramerateLimit(60);
    }

    sf::RenderWindow window;
    sf::Texture terrainTileset;
    detail::TerrainTilesetMetadata terrainTilesetMetadata;
    sf::Texture playerSpritesheet;
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
    sf::View view;
    view.setCenter({renderSnapshot.cameraFrame.center.x, renderSnapshot.cameraFrame.center.y});
    view.setSize({renderSnapshot.cameraFrame.size.width, renderSnapshot.cameraFrame.size.height});
    m_impl->window.setView(view);

    sf::Sprite tileSprite(m_impl->terrainTileset);
    const VisibleTileTypeMap visibleTileTypes = buildVisibleTileTypeMap(renderSnapshot);
    const std::uint32_t worldGenerationSeed = m_impl->overworldRuntime.getWorldGenerationSeed();
    sf::RectangleShape generatedContentShape;
    sf::Sprite playerSprite(m_impl->playerSpritesheet);
    sf::RectangleShape gridSegment;
    gridSegment.setFillColor(kGridOverlayColor);

    detail::executeOverworldRenderPasses(
        [&]()
        {
            for (const OverworldRenderTile& visibleTile : renderSnapshot.visibleTiles)
            {
                const float scaleX = visibleTile.size.width / static_cast<float>(kTerrainTilesetCellSize);
                const float scaleY = visibleTile.size.height / static_cast<float>(kTerrainTilesetCellSize);
                tileSprite.setTextureRect(getTerrainTilesetRect(detail::selectTerrainAtlasCell(
                    m_impl->terrainTilesetMetadata,
                    visibleTile,
                    getNeighborTileTypes(visibleTileTypes, visibleTile),
                    m_impl->terrainAnimationElapsedSeconds,
                    worldGenerationSeed)));
                tileSprite.setScale({scaleX, scaleY});
                tileSprite.setOrigin({visibleTile.origin.x / scaleX, visibleTile.origin.y / scaleY});
                tileSprite.setPosition({visibleTile.position.x, visibleTile.position.y});
                m_impl->window.draw(tileSprite);
            }
        },
        [&]()
        {
            for (const OverworldRenderContent& renderContent : renderSnapshot.generatedContent)
            {
                generatedContentShape.setSize({renderContent.size.width, renderContent.size.height});
                generatedContentShape.setOrigin({renderContent.origin.x, renderContent.origin.y});
                generatedContentShape.setPosition({renderContent.position.x, renderContent.position.y});
                generatedContentShape.setFillColor(getGeneratedContentColor(renderContent));
                m_impl->window.draw(generatedContentShape);
            }
        },
        [&]()
        {
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
        },
        [&]()
        {
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
        },
        detail::shouldRenderTileGridOverlay(m_impl->debugViewState));

    m_impl->window.display();
}

} // namespace rpg
