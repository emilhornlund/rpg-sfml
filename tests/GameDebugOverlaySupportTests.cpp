/**
 * @file GameDebugOverlaySupportTests.cpp
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

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/View.hpp>

#include <cmath>
#include <filesystem>

namespace
{

[[nodiscard]] bool verifyDebugOverlayRuntimeInitialization(const std::filesystem::path& assetRoot)
{
    const rpg::detail::DebugOverlayRuntime runtime = rpg::detail::makeDebugOverlayRuntime(assetRoot);

    return !runtime.font.getInfo().family.empty()
        && !rpg::detail::shouldRenderDebugOverlay(runtime.state)
        && std::fabs(runtime.frameRateAccumulatedSeconds) < 0.0001F
        && runtime.frameRateSampleCount == 0U
        && runtime.displayedFramesPerSecond == 0;
}

[[nodiscard]] bool verifyDebugOverlayFrameRateSampling()
{
    rpg::detail::DebugOverlayRuntime runtime;

    rpg::detail::updateDebugOverlayRuntime(runtime, 0.1F);

    if (std::fabs(runtime.frameRateAccumulatedSeconds - 0.1F) >= 0.0001F
        || runtime.frameRateSampleCount != 1U
        || runtime.displayedFramesPerSecond != 10)
    {
        return false;
    }

    rpg::detail::updateDebugOverlayRuntime(runtime, 0.1F);

    if (std::fabs(runtime.frameRateAccumulatedSeconds - 0.2F) >= 0.0001F
        || runtime.frameRateSampleCount != 2U
        || runtime.displayedFramesPerSecond != 10)
    {
        return false;
    }

    rpg::detail::updateDebugOverlayRuntime(runtime, 0.05F);

    return std::fabs(runtime.frameRateAccumulatedSeconds) < 0.0001F
        && runtime.frameRateSampleCount == 0U
        && runtime.displayedFramesPerSecond == 12;
}

[[nodiscard]] bool verifyDebugOverlayLayout()
{
    const sf::FloatRect textBounds{{-2.0F, 4.0F}, {100.0F, 28.0F}};
    const rpg::detail::DebugOverlayLayout layout = rpg::detail::makeDebugOverlayLayout(textBounds);

    return std::fabs(layout.textPosition.x - 22.0F) < 0.0001F
        && std::fabs(layout.textPosition.y - 16.0F) < 0.0001F
        && std::fabs(layout.backgroundPosition.x - 12.0F) < 0.0001F
        && std::fabs(layout.backgroundPosition.y - 12.0F) < 0.0001F
        && std::fabs(layout.backgroundSize.x - 116.0F) < 0.0001F
        && std::fabs(layout.backgroundSize.y - 44.0F) < 0.0001F;
}

[[nodiscard]] bool verifyDebugOverlayDrawVisibility(const std::filesystem::path& assetRoot)
{
    constexpr sf::Vector2u renderTargetSize{256U, 256U};
    sf::RenderTexture renderTexture;

    if (!renderTexture.resize(renderTargetSize))
    {
        return false;
    }

    const sf::View overlayView{{128.0F, 128.0F}, {256.0F, 256.0F}};
    const rpg::OverworldDebugSnapshot debugSnapshot{
        {12, -7},
        150,
        17,
        42,
        256,
        11};
    const rpg::detail::DebugOverlayRenderMetrics renderMetrics{
        3,
        2,
        1536,
        36};

    rpg::detail::DebugOverlayRuntime hiddenRuntime = rpg::detail::makeDebugOverlayRuntime(assetRoot);
    renderTexture.clear(sf::Color::Transparent);
    rpg::detail::drawDebugOverlay(renderTexture, overlayView, hiddenRuntime, debugSnapshot, renderMetrics);
    renderTexture.display();
    const sf::Image hiddenImage = renderTexture.getTexture().copyToImage();

    if (hiddenImage.getPixel({14U, 14U}).a != 0U)
    {
        return false;
    }

    rpg::detail::DebugOverlayRuntime visibleRuntime = rpg::detail::makeDebugOverlayRuntime(assetRoot);
    rpg::detail::toggleDebugOverlayVisibility(visibleRuntime.state);
    visibleRuntime.displayedFramesPerSecond = 144;
    renderTexture.clear(sf::Color::Transparent);
    rpg::detail::drawDebugOverlay(renderTexture, overlayView, visibleRuntime, debugSnapshot, renderMetrics);
    renderTexture.display();
    const sf::Image visibleImage = renderTexture.getTexture().copyToImage();

    return visibleImage.getPixel({14U, 14U}).a != 0U;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 1;
    }

    const std::filesystem::path assetRoot = argv[1];

    if (!verifyDebugOverlayRuntimeInitialization(assetRoot))
    {
        return 1;
    }

    if (!verifyDebugOverlayFrameRateSampling())
    {
        return 1;
    }

    if (!verifyDebugOverlayLayout())
    {
        return 1;
    }

    if (!verifyDebugOverlayDrawVisibility(assetRoot))
    {
        return 1;
    }

    return 0;
}
