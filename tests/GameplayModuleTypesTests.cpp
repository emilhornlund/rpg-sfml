/**
 * @file GameplayModuleTypesTests.cpp
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
#include <main/OverworldRuntime.hpp>
#include <main/Player.hpp>
#include <main/World.hpp>

#include <type_traits>

static_assert(std::is_default_constructible_v<rpg::World>);
static_assert(std::is_constructible_v<rpg::World, rpg::WorldConfig>);
static_assert(std::is_default_constructible_v<rpg::Player>);
static_assert(std::is_default_constructible_v<rpg::Camera>);
static_assert(std::is_default_constructible_v<rpg::OverworldRuntime>);

int main()
{
    rpg::World world;
    rpg::World configuredWorld(rpg::WorldConfig{});
    rpg::Player player;
    rpg::Camera camera;
    rpg::OverworldRuntime overworldRuntime;

    (void)world;
    (void)configuredWorld;
    (void)player;
    (void)camera;
    (void)overworldRuntime;

    return 0;
}
