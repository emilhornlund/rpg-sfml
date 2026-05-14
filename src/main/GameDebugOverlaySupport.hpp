/**
 * @file GameDebugOverlaySupport.hpp
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

#ifndef RPG_MAIN_GAME_DEBUG_OVERLAY_SUPPORT_HPP
#define RPG_MAIN_GAME_DEBUG_OVERLAY_SUPPORT_HPP

#include "GameRuntimeSupport.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <filesystem>

namespace rpg::detail
{

struct DebugOverlayRuntime
{
    sf::Font font;
    DebugOverlayState state;
    float frameRateAccumulatedSeconds = 0.0F;
    std::uint32_t frameRateSampleCount = 0;
    int displayedFramesPerSecond = 0;
};

struct DebugOverlayLayout
{
    sf::Vector2f textPosition{0.0F, 0.0F};
    sf::Vector2f backgroundPosition{0.0F, 0.0F};
    sf::Vector2f backgroundSize{0.0F, 0.0F};
};

[[nodiscard]] sf::Font loadDebugOverlayFont(const std::filesystem::path& assetRoot);

[[nodiscard]] sf::Font loadDebugOverlayFont();

[[nodiscard]] DebugOverlayRuntime makeDebugOverlayRuntime(const std::filesystem::path& assetRoot);

[[nodiscard]] DebugOverlayRuntime makeDebugOverlayRuntime();

void updateDebugOverlayRuntime(DebugOverlayRuntime& debugOverlayRuntime, float deltaTimeSeconds) noexcept;

[[nodiscard]] DebugOverlayLayout makeDebugOverlayLayout(const sf::FloatRect& textBounds) noexcept;

void drawDebugOverlay(
    sf::RenderTarget& target,
    const sf::View& overlayView,
    const DebugOverlayRuntime& debugOverlayRuntime,
    const OverworldDebugSnapshot& debugSnapshot,
    const DebugOverlayRenderMetrics& renderMetrics);

} // namespace rpg::detail

#endif // RPG_MAIN_GAME_DEBUG_OVERLAY_SUPPORT_HPP
