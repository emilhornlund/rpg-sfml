/**
 * @file OverworldTypes.hpp
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

#ifndef RPG_MAIN_OVERWORLD_TYPES_HPP
#define RPG_MAIN_OVERWORLD_TYPES_HPP

/**
 * @brief Core runtime types for the RPG executable.
 */
namespace rpg
{

/**
 * @brief Tile-space coordinates inside the overworld grid.
 */
struct TileCoordinates
{
    int x;
    int y;
};

/**
 * @brief Position in world-space units.
 */
struct WorldPosition
{
    float x;
    float y;
};

/**
 * @brief Width and height in world-space units.
 */
struct WorldSize
{
    float width;
    float height;
};

/**
 * @brief View framing data for the camera.
 */
struct ViewFrame
{
    WorldPosition center;
    WorldSize size;
};

/**
 * @brief Normalized movement intent for the player update step.
 */
struct MovementIntent
{
    float x;
    float y;
};

} // namespace rpg

#endif // RPG_MAIN_OVERWORLD_TYPES_HPP
