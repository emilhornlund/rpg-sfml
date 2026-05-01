/**
 * @file OverworldRuntime.hpp
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

#ifndef RPG_MAIN_OVERWORLD_RUNTIME_HPP
#define RPG_MAIN_OVERWORLD_RUNTIME_HPP

#include <main/Camera.hpp>
#include <main/Player.hpp>
#include <main/World.hpp>

#include <vector>

/**
 * @brief Core runtime types for the RPG executable.
 */
namespace rpg
{

/**
 * @brief Per-frame overworld input supplied by the runtime shell.
 */
struct OverworldInput
{
    MovementIntent movementIntent{0.0F, 0.0F};
    WorldSize viewportSize{0.0F, 0.0F};
};

/**
 * @brief Presentation identifiers for renderable overworld markers.
 */
enum class OverworldRenderMarkerAppearance
{
    Player
};

/**
 * @brief Render-ready tile entry for the overworld view.
 */
struct OverworldRenderTile
{
    TileCoordinates coordinates{0, 0};
    TileType tileType = TileType::Water;
    WorldSize size{0.0F, 0.0F};
    WorldPosition origin{0.0F, 0.0F};
    WorldPosition position{0.0F, 0.0F};
};

/**
 * @brief Render-ready marker entry for gameplay entities in the overworld view.
 */
struct OverworldRenderMarker
{
    WorldSize size{0.0F, 0.0F};
    WorldPosition origin{0.0F, 0.0F};
    WorldPosition position{0.0F, 0.0F};
    OverworldRenderMarkerAppearance appearance = OverworldRenderMarkerAppearance::Player;
    PlayerFacingDirection facingDirection = PlayerFacingDirection::Down;
    int animationFrameIndex = 1;
};

/**
 * @brief Gameplay-owned render snapshot for the current overworld frame.
 */
struct OverworldRenderSnapshot
{
    ViewFrame cameraFrame{{0.0F, 0.0F}, {0.0F, 0.0F}};
    std::vector<OverworldRenderTile> visibleTiles;
    std::vector<OverworldRenderMarker> markers;
};

/**
 * @brief Owns the active overworld gameplay session.
 *
 * The overworld runtime coordinates the world, player, and camera modules for
 * the current gameplay slice. It accepts repo-native input from the outer
 * runtime shell and exposes render-facing state for the active frame without
 * leaking SFML types into gameplay code.
 */
class OverworldRuntime
{
public:
    /**
     * @brief Construct a new overworld runtime.
     */
    OverworldRuntime();

    /**
     * @brief Destroy the overworld runtime.
     */
    ~OverworldRuntime();

    /**
     * @brief Initialize the active overworld session for a viewport.
     *
     * Initializes the player at the world-provided spawn position, updates the
     * camera framing for the provided viewport, and prepares the initial render
     * snapshot. Repeated calls refresh the snapshot for the current viewport.
     *
     * @param viewportSize Active viewport size in world-space units.
     */
    void initialize(const WorldSize& viewportSize);

    /**
     * @brief Advance the overworld simulation for one frame.
     *
     * Applies the current frame's input, advances gameplay modules, and refreshes
     * the render snapshot for the current frame.
     *
     * @param deltaTimeSeconds Elapsed frame time in seconds.
     * @param input Overworld input and viewport data for the frame.
     */
    void update(float deltaTimeSeconds, const OverworldInput& input);

    /**
     * @brief Read the render snapshot for the active overworld frame.
     *
     * @return Current render snapshot.
     */
    [[nodiscard]] const OverworldRenderSnapshot& getRenderSnapshot() const noexcept;

private:
    /**
     * @brief Refresh the render snapshot from the active gameplay modules.
     */
    void refreshRenderSnapshot();

    World m_world;
    Player m_player;
    Camera m_camera;
    OverworldRenderSnapshot m_renderSnapshot;
    bool m_isInitialized = false;
};

} // namespace rpg

#endif // RPG_MAIN_OVERWORLD_RUNTIME_HPP
