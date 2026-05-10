/**
 * @file PlayerOcclusionSilhouetteSupport.hpp
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

#ifndef RPG_MAIN_PLAYER_OCCLUSION_SILHOUETTE_SUPPORT_HPP
#define RPG_MAIN_PLAYER_OCCLUSION_SILHOUETTE_SUPPORT_HPP

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <stdexcept>
#include <string_view>

namespace rpg::detail
{

inline constexpr sf::Color kPlayerOcclusionSilhouetteColor(196, 223, 255, 176);
inline constexpr std::string_view kPlayerOcclusionSilhouetteShaderSource = R"(
uniform sampler2D currentTexture;
uniform sampler2D occluderMask;
uniform vec4 silhouetteColor;

void main()
{
    vec2 coordinates = gl_TexCoord[0].xy;
    vec4 playerPixel = texture2D(currentTexture, coordinates);
    vec4 occluderPixel = texture2D(occluderMask, coordinates);
    float alpha = playerPixel.a * occluderPixel.a * silhouetteColor.a;
    gl_FragColor = vec4(silhouetteColor.rgb, alpha);
}
)";

[[nodiscard]] inline sf::Shader loadPlayerOcclusionShader()
{
    if (!sf::Shader::isAvailable())
    {
        throw std::runtime_error("Failed to initialize player occlusion silhouette shader: shaders are unavailable.");
    }

    return {kPlayerOcclusionSilhouetteShaderSource, sf::Shader::Type::Fragment};
}

} // namespace rpg::detail

#endif // RPG_MAIN_PLAYER_OCCLUSION_SILHOUETTE_SUPPORT_HPP
