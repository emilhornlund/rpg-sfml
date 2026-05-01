/**
 * @file Camera.cpp
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

#include <main/Camera.hpp>

namespace rpg
{

namespace
{

[[nodiscard]] constexpr int clampZoomPercent(const int zoomPercent) noexcept
{
    constexpr int kMinZoomPercent = 100;
    constexpr int kMaxZoomPercent = 400;
    return zoomPercent < kMinZoomPercent
        ? kMinZoomPercent
        : (zoomPercent > kMaxZoomPercent ? kMaxZoomPercent : zoomPercent);
}

} // namespace

Camera::Camera() = default;

Camera::~Camera() = default;

void Camera::update(
    const WorldPosition& focusPosition,
    const float viewportWidth,
    const float viewportHeight) noexcept
{
    const float zoomScale = 100.0F / static_cast<float>(m_state.zoomPercent);
    m_state.frame.size = {viewportWidth * zoomScale, viewportHeight * zoomScale};
    m_state.frame.center = focusPosition;
}

void Camera::setZoomPercent(const int zoomPercent) noexcept
{
    m_state.zoomPercent = clampZoomPercent(zoomPercent);
}

int Camera::getZoomPercent() const noexcept
{
    return m_state.zoomPercent;
}

ViewFrame Camera::getFrame() const noexcept
{
    return m_state.frame;
}

} // namespace rpg
