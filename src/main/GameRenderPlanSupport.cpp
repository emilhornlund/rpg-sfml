/**
 * @file GameRenderPlanSupport.cpp
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

#include "GameRenderPlanSupport.hpp"

#include <algorithm>

namespace rpg
{
namespace detail
{

std::vector<OverworldRenderQueueEntry> buildOverworldRenderQueue(const OverworldRenderSnapshot& renderSnapshot)
{
    std::vector<OverworldRenderQueueEntry> renderQueue;
    renderQueue.reserve(renderSnapshot.generatedContent.size() + renderSnapshot.markers.size());

    for (std::size_t index = 0; index < renderSnapshot.generatedContent.size(); ++index)
    {
        renderQueue.push_back(makeRenderQueueEntry(renderSnapshot.generatedContent[index], index));
    }

    for (std::size_t index = 0; index < renderSnapshot.markers.size(); ++index)
    {
        renderQueue.push_back(makeRenderQueueEntry(renderSnapshot.markers[index], index));
    }

    std::stable_sort(
        renderQueue.begin(),
        renderQueue.end(),
        [](const OverworldRenderQueueEntry& lhs, const OverworldRenderQueueEntry& rhs)
        {
            return shouldRenderBefore(lhs.orderKey, rhs.orderKey);
        });

    return renderQueue;
}

std::optional<std::size_t> findPlayerMarkerIndex(const OverworldRenderSnapshot& renderSnapshot) noexcept
{
    const auto playerMarkerIt = std::find_if(
        renderSnapshot.markers.begin(),
        renderSnapshot.markers.end(),
        [](const OverworldRenderMarker& renderMarker)
        {
            return renderMarker.appearance == OverworldRenderMarkerAppearance::Player;
        });

    if (playerMarkerIt == renderSnapshot.markers.end())
    {
        return std::nullopt;
    }

    return static_cast<std::size_t>(std::distance(renderSnapshot.markers.begin(), playerMarkerIt));
}

OverworldRenderPlan buildOverworldRenderPlan(
    const OverworldRenderSnapshot& renderSnapshot,
    const std::size_t terrainVertexCount,
    const std::size_t gridVertexCount)
{
    OverworldRenderPlan renderPlan;
    renderPlan.renderQueue = buildOverworldRenderQueue(renderSnapshot);
    renderPlan.playerMarkerIndex = findPlayerMarkerIndex(renderSnapshot);
    renderPlan.frontOccluderIndices = collectFrontGeneratedContentIndices(renderPlan.renderQueue);

    if (renderPlan.playerMarkerIndex.has_value())
    {
        renderPlan.overlapQualifiedOcclusionCandidateIndices = collectOverlapQualifiedFrontGeneratedContentIndices(
            renderPlan.frontOccluderIndices,
            renderSnapshot.generatedContent,
            renderSnapshot.markers[*renderPlan.playerMarkerIndex]);
    }

    renderPlan.debugOverlayRenderMetrics = {
        renderPlan.frontOccluderIndices.size(),
        renderPlan.overlapQualifiedOcclusionCandidateIndices.size(),
        terrainVertexCount,
        gridVertexCount,
    };

    return renderPlan;
}

} // namespace detail
} // namespace rpg
