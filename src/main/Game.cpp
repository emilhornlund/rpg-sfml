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

#include "GameAssetSupport.hpp"
#include "GameRuntimeSupport.hpp"
#include "PlayerOcclusionSilhouetteSupport.hpp"

#include <algorithm>
#include <SFML/Graphics/Font.hpp>
#include <limits>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>

#include <filesystem>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rpg
{
constexpr unsigned int kWindowWidth = 1280;
constexpr unsigned int kWindowHeight = 720;
constexpr int kTerrainTilesetCellSize = 16;
constexpr int kVegetationTilesetCellSize = 16;
constexpr int kPlayerSpritesheetCellSize = 48;
const sf::Color kBackgroundColor(24, 24, 27);
const sf::Color kGridOverlayColor(255, 255, 255, 96);
const sf::Color kDebugOverlayBackgroundColor(0, 0, 0, 160);
const sf::Color kDebugOverlayTextColor(245, 245, 245);
constexpr float kGridOverlayThickness = 1.0F;
constexpr float kDebugOverlayPadding = 8.0F;
constexpr float kDebugOverlayMargin = 12.0F;
constexpr unsigned int kDebugOverlayCharacterSize = 16U;
constexpr float kDebugOverlayFrameRateSampleWindowSeconds = 0.25F;
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

[[nodiscard]] sf::Font loadDebugOverlayFont()
{
    sf::Font debugOverlayFont;
    const std::filesystem::path debugOverlayFontPath = detail::getDebugOverlayFontPath(detail::getAssetRootPath());

    if (!debugOverlayFont.openFromFile(debugOverlayFontPath))
    {
        throw std::runtime_error("Failed to load debug overlay font from " + debugOverlayFontPath.string());
    }

    return debugOverlayFont;
}

[[nodiscard]] sf::Texture loadTerrainTileset()
{
    sf::Texture terrainTileset;
    const std::filesystem::path terrainTilesetPath = detail::getTerrainTilesetPath(detail::getAssetRootPath());

    if (!terrainTileset.loadFromFile(terrainTilesetPath.string()))
    {
        throw std::runtime_error("Failed to load overworld terrain tileset from " + terrainTilesetPath.string());
    }

    terrainTileset.setSmooth(false);
    return terrainTileset;
}

[[nodiscard]] detail::TerrainTilesetMetadata loadTerrainTilesetMetadata()
{
    return detail::loadTerrainTilesetMetadata(detail::getAssetRootPath());
}

[[nodiscard]] sf::Texture loadPlayerSpritesheet()
{
    sf::Texture playerSpritesheet;
    const std::filesystem::path playerSpritesheetPath = detail::getPlayerSpritesheetPath(detail::getAssetRootPath());

    if (!playerSpritesheet.loadFromFile(playerSpritesheetPath.string()))
    {
        throw std::runtime_error("Failed to load player spritesheet from " + playerSpritesheetPath.string());
    }

    playerSpritesheet.setSmooth(false);
    return playerSpritesheet;
}

[[nodiscard]] sf::Texture loadVegetationTileset()
{
    sf::Texture vegetationTileset;
    const std::filesystem::path vegetationTilesetPath = detail::getVegetationTilesetPath(detail::getAssetRootPath());

    if (!vegetationTileset.loadFromFile(vegetationTilesetPath.string()))
    {
        throw std::runtime_error("Failed to load overworld vegetation tileset from " + vegetationTilesetPath.string());
    }

    vegetationTileset.setSmooth(false);
    return vegetationTileset;
}

[[nodiscard]] detail::VegetationTilesetMetadata loadVegetationTilesetMetadata()
{
    return detail::loadVegetationTilesetMetadata(detail::getAssetRootPath());
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

[[nodiscard]] sf::IntRect getVegetationTilesetRect(const detail::VegetationAtlasCell& cell) noexcept
{
    return {
        {cell.tileX * kVegetationTilesetCellSize, cell.tileY * kVegetationTilesetCellSize},
        {kVegetationTilesetCellSize, kVegetationTilesetCellSize}};
}

[[nodiscard]] int getDisplayedFrameRate(
    const float accumulatedSeconds,
    const std::uint32_t sampleCount,
    const int fallbackFrameRate) noexcept
{
    return accumulatedSeconds > 0.0F
        ? static_cast<int>(static_cast<float>(sampleCount) / accumulatedSeconds + 0.5F)
        : fallbackFrameRate;
}

void ensureRenderTextureSize(sf::RenderTexture& renderTexture, const sf::Vector2u size)
{
    if (size.x == 0U || size.y == 0U)
    {
        throw std::runtime_error("Failed to initialize player occlusion silhouette render target: window size is invalid.");
    }

    const sf::Vector2u currentSize = renderTexture.getSize();

    if (currentSize.x != size.x || currentSize.y != size.y)
    {
        if (!renderTexture.resize(size))
        {
            throw std::runtime_error("Failed to resize player occlusion silhouette render target.");
        }

        renderTexture.setSmooth(false);
    }
}

void drawVegetationContent(
    sf::RenderTarget& target,
    sf::Sprite& vegetationSprite,
    const detail::VegetationTilesetMetadata& vegetationTilesetMetadata,
    const OverworldRenderContent& renderContent,
    const float worldTileSize)
{
    const detail::VegetationPrototype& prototype = vegetationTilesetMetadata.getPrototypeById(renderContent.prototypeId);

    for (const detail::VegetationAtlasPart& part : prototype.parts)
    {
        const float scaleX = worldTileSize / static_cast<float>(kVegetationTilesetCellSize);
        const float scaleY = worldTileSize / static_cast<float>(kVegetationTilesetCellSize);
        vegetationSprite.setTextureRect(getVegetationTilesetRect(part.cell));
        vegetationSprite.setScale({scaleX, scaleY});
        vegetationSprite.setOrigin({8.0F, 8.0F});
        vegetationSprite.setPosition({
            renderContent.position.x + static_cast<float>(part.offsetX) * worldTileSize,
            renderContent.position.y + static_cast<float>(part.offsetY) * worldTileSize});
        target.draw(vegetationSprite);
    }
}

void drawPlayerMarker(
    sf::RenderTarget& target,
    sf::Sprite& sprite,
    const OverworldRenderMarker& renderMarker)
{
    const float scaleX = renderMarker.size.width / static_cast<float>(kPlayerSpritesheetCellSize);
    const float scaleY = renderMarker.size.height / static_cast<float>(kPlayerSpritesheetCellSize);
    sprite.setTextureRect(getPlayerSpritesheetRect(renderMarker));
    sprite.setScale({scaleX, scaleY});
    sprite.setOrigin({renderMarker.origin.x / scaleX, renderMarker.origin.y / scaleY});
    sprite.setPosition({renderMarker.position.x, renderMarker.position.y});
    target.draw(sprite);
}

class Game::Impl
{
public:
    Impl()
        : window(sf::VideoMode({kWindowWidth, kWindowHeight}), "rpg-sfml", sf::State::Windowed)
        , terrainTileset(loadTerrainTileset())
        , terrainTilesetMetadata(loadTerrainTilesetMetadata())
        , vegetationTileset(loadVegetationTileset())
        , vegetationTilesetMetadata(loadVegetationTilesetMetadata())
        , playerSpritesheet(loadPlayerSpritesheet())
        , playerOcclusionShader(detail::loadPlayerOcclusionShader())
        , debugOverlayFont(loadDebugOverlayFont())
    {
        const detail::WindowFramePacingConfig framePacing = detail::getDefaultWindowFramePacingConfig();
        window.setVerticalSyncEnabled(framePacing.mode == detail::WindowFramePacingMode::VerticalSync);
        window.setFramerateLimit(
            framePacing.mode == detail::WindowFramePacingMode::FramerateLimit ? framePacing.framerateLimit : 0U);
    }

    sf::RenderWindow window;
    sf::Texture terrainTileset;
    detail::TerrainTilesetMetadata terrainTilesetMetadata;
    sf::Texture vegetationTileset;
    detail::VegetationTilesetMetadata vegetationTilesetMetadata;
    sf::Texture playerSpritesheet;
    sf::RenderTexture playerOcclusionMaskTexture;
    sf::RenderTexture occluderMaskTexture;
    sf::Shader playerOcclusionShader;
    sf::Font debugOverlayFont;
    OverworldRuntime overworldRuntime;
    detail::OverworldDirectionalInput directionalInput;
    OverworldInput::DebugViewState debugViewState = detail::makeOverworldDebugViewState(detail::isDebugViewModeEnabledForBuild());
    detail::DebugOverlayState debugOverlayState;
    float terrainAnimationElapsedSeconds = 0.0F;
    float frameRateAccumulatedSeconds = 0.0F;
    std::uint32_t frameRateSampleCount = 0;
    int displayedFramesPerSecond = 0;
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

            if (keyPressed->code == sf::Keyboard::Key::F1)
            {
                detail::toggleDebugOverlayVisibility(m_impl->debugOverlayState);
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
    const sf::Vector2u windowSize = m_impl->window.getSize();
    const WorldSize viewportSize = detail::makeViewportSize(windowSize.x, windowSize.y);
    m_impl->frameRateAccumulatedSeconds += std::max(deltaTimeSeconds, 0.0F);
    ++m_impl->frameRateSampleCount;

    if (m_impl->frameRateAccumulatedSeconds >= kDebugOverlayFrameRateSampleWindowSeconds)
    {
        m_impl->displayedFramesPerSecond = getDisplayedFrameRate(
            m_impl->frameRateAccumulatedSeconds,
            m_impl->frameRateSampleCount,
            m_impl->displayedFramesPerSecond);
        m_impl->frameRateAccumulatedSeconds = 0.0F;
        m_impl->frameRateSampleCount = 0;
    }
    else if (m_impl->displayedFramesPerSecond == 0)
    {
        m_impl->displayedFramesPerSecond = getDisplayedFrameRate(
            m_impl->frameRateAccumulatedSeconds,
            m_impl->frameRateSampleCount,
            0);
    }

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
    view.setCenter({renderSnapshot.cameraFrame.center.x, renderSnapshot.cameraFrame.center.y});
    view.setSize({renderSnapshot.cameraFrame.size.width, renderSnapshot.cameraFrame.size.height});
    m_impl->window.setView(view);

    sf::Sprite tileSprite(m_impl->terrainTileset);
    const VisibleTileTypeMap visibleTileTypes = buildVisibleTileTypeMap(renderSnapshot);
    const std::uint32_t worldGenerationSeed = m_impl->overworldRuntime.getWorldGenerationSeed();
    const float worldTileSize = renderSnapshot.visibleTiles.empty() ? 16.0F : renderSnapshot.visibleTiles.front().size.width;
    sf::Sprite vegetationSprite(m_impl->vegetationTileset);
    sf::Sprite playerSprite(m_impl->playerSpritesheet);
    sf::RectangleShape gridSegment;
    gridSegment.setFillColor(kGridOverlayColor);

    std::vector<detail::OverworldRenderQueueEntry> renderQueue;
    renderQueue.reserve(renderSnapshot.generatedContent.size() + renderSnapshot.markers.size());

    for (std::size_t index = 0; index < renderSnapshot.generatedContent.size(); ++index)
    {
        renderQueue.push_back(detail::makeRenderQueueEntry(renderSnapshot.generatedContent[index], index));
    }

    for (std::size_t index = 0; index < renderSnapshot.markers.size(); ++index)
    {
        const OverworldRenderMarker& renderMarker = renderSnapshot.markers[index];
        renderQueue.push_back(detail::makeRenderQueueEntry(renderMarker, index));
    }

    std::stable_sort(
        renderQueue.begin(),
        renderQueue.end(),
        [](const detail::OverworldRenderQueueEntry& lhs, const detail::OverworldRenderQueueEntry& rhs)
        {
            return detail::shouldRenderBefore(lhs.orderKey, rhs.orderKey);
        });

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
            for (const detail::OverworldRenderQueueEntry& entry : renderQueue)
            {
                if (entry.kind == detail::OverworldRenderQueueEntryKind::GeneratedContent)
                {
                    drawVegetationContent(
                        m_impl->window,
                        vegetationSprite,
                        m_impl->vegetationTilesetMetadata,
                        renderSnapshot.generatedContent[entry.sourceIndex],
                        worldTileSize);
                    continue;
                }

                const OverworldRenderMarker& renderMarker = renderSnapshot.markers[entry.sourceIndex];
                const float scaleX = renderMarker.size.width / static_cast<float>(kPlayerSpritesheetCellSize);
                const float scaleY = renderMarker.size.height / static_cast<float>(kPlayerSpritesheetCellSize);
                playerSprite.setTextureRect(getPlayerSpritesheetRect(renderMarker));
                playerSprite.setScale({scaleX, scaleY});
                playerSprite.setOrigin({renderMarker.origin.x / scaleX, renderMarker.origin.y / scaleY});
                playerSprite.setPosition({renderMarker.position.x, renderMarker.position.y});
                m_impl->window.draw(playerSprite);
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
        detail::shouldRenderTileGridOverlay(m_impl->debugViewState));

    const std::vector<std::size_t> frontOccluderIndices = detail::collectFrontGeneratedContentIndices(renderQueue);
    const auto playerMarkerIt = std::find_if(
        renderSnapshot.markers.begin(),
        renderSnapshot.markers.end(),
        [](const OverworldRenderMarker& renderMarker)
        {
            return renderMarker.appearance == OverworldRenderMarkerAppearance::Player;
        });

    if (playerMarkerIt != renderSnapshot.markers.end() && !frontOccluderIndices.empty())
    {
        const sf::Vector2u windowSize = m_impl->window.getSize();
        ensureRenderTextureSize(m_impl->playerOcclusionMaskTexture, windowSize);
        ensureRenderTextureSize(m_impl->occluderMaskTexture, windowSize);

        m_impl->playerOcclusionMaskTexture.setView(view);
        m_impl->occluderMaskTexture.setView(view);
        m_impl->playerOcclusionMaskTexture.clear(sf::Color::Transparent);
        m_impl->occluderMaskTexture.clear(sf::Color::Transparent);

        drawPlayerMarker(m_impl->playerOcclusionMaskTexture, playerSprite, *playerMarkerIt);

        for (const std::size_t index : frontOccluderIndices)
        {
            drawVegetationContent(
                m_impl->occluderMaskTexture,
                vegetationSprite,
                m_impl->vegetationTilesetMetadata,
                renderSnapshot.generatedContent[index],
                worldTileSize);
        }

        m_impl->playerOcclusionMaskTexture.display();
        m_impl->occluderMaskTexture.display();

        sf::Sprite playerOcclusionSprite(m_impl->playerOcclusionMaskTexture.getTexture());
        m_impl->playerOcclusionShader.setUniform("currentTexture", sf::Shader::CurrentTexture);
        m_impl->playerOcclusionShader.setUniform("occluderMask", m_impl->occluderMaskTexture.getTexture());
        m_impl->playerOcclusionShader.setUniform(
            "silhouetteColor",
            sf::Glsl::Vec4(detail::kPlayerOcclusionSilhouetteColor));

        m_impl->window.setView(m_impl->window.getDefaultView());
        m_impl->window.draw(playerOcclusionSprite, &m_impl->playerOcclusionShader);
        m_impl->window.setView(view);
    }

    if (detail::shouldRenderDebugOverlay(m_impl->debugOverlayState))
    {
        m_impl->window.setView(m_impl->window.getDefaultView());

        sf::Text debugOverlayText(
            m_impl->debugOverlayFont,
            detail::buildDebugOverlayString(
                debugSnapshot,
                m_impl->displayedFramesPerSecond),
            kDebugOverlayCharacterSize);
        debugOverlayText.setFillColor(kDebugOverlayTextColor);

        const sf::FloatRect textBounds = debugOverlayText.getLocalBounds();
        debugOverlayText.setPosition({
            kDebugOverlayMargin + kDebugOverlayPadding - textBounds.position.x,
            kDebugOverlayMargin + kDebugOverlayPadding - textBounds.position.y});
        sf::RectangleShape debugOverlayBackground;
        debugOverlayBackground.setFillColor(kDebugOverlayBackgroundColor);
        debugOverlayBackground.setPosition({kDebugOverlayMargin, kDebugOverlayMargin});
        debugOverlayBackground.setSize({
            textBounds.size.x + (kDebugOverlayPadding * 2.0F),
            textBounds.size.y + (kDebugOverlayPadding * 2.0F)});

        m_impl->window.draw(debugOverlayBackground);
        m_impl->window.draw(debugOverlayText);
    }

    m_impl->window.display();
}

} // namespace rpg
