/**
 * @file Camera.hpp
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

#ifndef RPG_MAIN_CAMERA_HPP
#define RPG_MAIN_CAMERA_HPP

#include <main/OverworldTypes.hpp>

/**
 * @brief Core runtime types for the RPG executable.
 */
namespace rpg
{

/**
 * @brief Owns camera-specific runtime state.
 *
 * The Camera class provides the dedicated boundary for camera-facing gameplay
 * data and behavior. It gives the runtime a stable type where future view
 * tracking, following, and framing logic can evolve without being declared in
 * the top-level game loop coordinator.
 */
class Camera
{
public:
    /**
     * @brief Construct a new camera module.
     *
     * Prepares the camera-owned state used by the runtime.
     */
    Camera();

    /**
     * @brief Destroy the camera module.
     *
     * Releases any resources retained by the camera-owned state.
     */
    ~Camera();

    /**
     * @brief Update the camera frame from the player position.
     *
     * @param focusPosition World-space position to follow.
     * @param viewportWidth Desired viewport width in world units.
     * @param viewportHeight Desired viewport height in world units.
     */
    void update(
        const WorldPosition& focusPosition,
        float viewportWidth,
        float viewportHeight) noexcept;

    /**
     * @brief Set the active camera zoom level.
     *
     * The zoom level is expressed as a percentage where 100 keeps the
     * viewport-sized frame unchanged and larger values zoom in by publishing a
     * smaller camera frame.
     *
     * @param zoomPercent Zoom level percentage.
     */
    void setZoomPercent(int zoomPercent) noexcept;

    /**
     * @brief Read the active camera zoom level.
     *
     * @return Zoom level percentage.
     */
    [[nodiscard]] int getZoomPercent() const noexcept;

    /**
     * @brief Read the current camera frame.
     *
     * @return The current frame center and size.
     */
    [[nodiscard]] ViewFrame getFrame() const noexcept;

private:
    /**
     * @brief Internal camera state container.
     *
     * The nested state keeps the public type ready for future internal
     * expansion without exposing implementation details in the header.
     */
    struct State
    {
        ViewFrame frame{{0.0F, 0.0F}, {0.0F, 0.0F}};
        int zoomPercent = 300;
    };

    /**
     * @brief Storage for camera-owned runtime state.
     */
    State m_state;
};

} // namespace rpg

#endif // RPG_MAIN_CAMERA_HPP
