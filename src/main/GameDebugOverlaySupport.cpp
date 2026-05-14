/**
 * @file GameDebugOverlaySupport.cpp
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

#include "GameDebugOverlaySupport.hpp"

#include "GameAssetSupport.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <stdexcept>

namespace rpg
{
namespace detail
{
namespace
{

const sf::Color kDebugOverlayBackgroundColor(0, 0, 0, 160);
const sf::Color kDebugOverlayTextColor(245, 245, 245);
constexpr float kDebugOverlayPadding = 8.0F;
constexpr float kDebugOverlayMargin = 12.0F;
constexpr unsigned int kDebugOverlayCharacterSize = 16U;
constexpr float kDebugOverlayFrameRateSampleWindowSeconds = 0.25F;

[[nodiscard]] int getDisplayedFrameRate(
    const float accumulatedSeconds,
    const std::uint32_t sampleCount,
    const int fallbackFrameRate) noexcept
{
    return accumulatedSeconds > 0.0F
        ? static_cast<int>(static_cast<float>(sampleCount) / accumulatedSeconds + 0.5F)
        : fallbackFrameRate;
}

} // namespace

sf::Font loadDebugOverlayFont(const std::filesystem::path& assetRoot)
{
    sf::Font debugOverlayFont;
    const std::filesystem::path debugOverlayFontPath = getDebugOverlayFontPath(assetRoot);

    if (!debugOverlayFont.openFromFile(debugOverlayFontPath))
    {
        throw std::runtime_error("Failed to load debug overlay font from " + debugOverlayFontPath.string());
    }

    return debugOverlayFont;
}

sf::Font loadDebugOverlayFont()
{
    return loadDebugOverlayFont(getAssetRootPath());
}

DebugOverlayRuntime makeDebugOverlayRuntime(const std::filesystem::path& assetRoot)
{
    DebugOverlayRuntime debugOverlayRuntime;
    debugOverlayRuntime.font = loadDebugOverlayFont(assetRoot);
    return debugOverlayRuntime;
}

DebugOverlayRuntime makeDebugOverlayRuntime()
{
    return makeDebugOverlayRuntime(getAssetRootPath());
}

void updateDebugOverlayRuntime(DebugOverlayRuntime& debugOverlayRuntime, const float deltaTimeSeconds) noexcept
{
    debugOverlayRuntime.frameRateAccumulatedSeconds += std::max(deltaTimeSeconds, 0.0F);
    ++debugOverlayRuntime.frameRateSampleCount;

    if (debugOverlayRuntime.frameRateAccumulatedSeconds >= kDebugOverlayFrameRateSampleWindowSeconds)
    {
        debugOverlayRuntime.displayedFramesPerSecond = getDisplayedFrameRate(
            debugOverlayRuntime.frameRateAccumulatedSeconds,
            debugOverlayRuntime.frameRateSampleCount,
            debugOverlayRuntime.displayedFramesPerSecond);
        debugOverlayRuntime.frameRateAccumulatedSeconds = 0.0F;
        debugOverlayRuntime.frameRateSampleCount = 0;
    }
    else if (debugOverlayRuntime.displayedFramesPerSecond == 0)
    {
        debugOverlayRuntime.displayedFramesPerSecond = getDisplayedFrameRate(
            debugOverlayRuntime.frameRateAccumulatedSeconds,
            debugOverlayRuntime.frameRateSampleCount,
            0);
    }
}

DebugOverlayLayout makeDebugOverlayLayout(const sf::FloatRect& textBounds) noexcept
{
    return {
        {
            kDebugOverlayMargin + kDebugOverlayPadding - textBounds.position.x,
            kDebugOverlayMargin + kDebugOverlayPadding - textBounds.position.y,
        },
        {kDebugOverlayMargin, kDebugOverlayMargin},
        {
            textBounds.size.x + (kDebugOverlayPadding * 2.0F),
            textBounds.size.y + (kDebugOverlayPadding * 2.0F),
        },
    };
}

void drawDebugOverlay(
    sf::RenderTarget& target,
    const sf::View& overlayView,
    const DebugOverlayRuntime& debugOverlayRuntime,
    const OverworldDebugSnapshot& debugSnapshot,
    const DebugOverlayRenderMetrics& renderMetrics)
{
    if (!shouldRenderDebugOverlay(debugOverlayRuntime.state))
    {
        return;
    }

    target.setView(overlayView);

    sf::Text debugOverlayText(
        debugOverlayRuntime.font,
        buildDebugOverlayString(
            debugSnapshot,
            renderMetrics,
            debugOverlayRuntime.displayedFramesPerSecond),
        kDebugOverlayCharacterSize);
    debugOverlayText.setFillColor(kDebugOverlayTextColor);

    const DebugOverlayLayout layout = makeDebugOverlayLayout(debugOverlayText.getLocalBounds());
    debugOverlayText.setPosition(layout.textPosition);

    sf::RectangleShape debugOverlayBackground;
    debugOverlayBackground.setFillColor(kDebugOverlayBackgroundColor);
    debugOverlayBackground.setPosition(layout.backgroundPosition);
    debugOverlayBackground.setSize(layout.backgroundSize);

    target.draw(debugOverlayBackground);
    target.draw(debugOverlayText);
}

} // namespace detail
} // namespace rpg
