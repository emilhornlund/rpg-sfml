/**
 * @file GameRuntimeSupport.cpp
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

#include "GameRuntimeSupport.hpp"

#include <cmath>
#include <sstream>

namespace rpg
{
namespace detail
{
namespace
{

[[nodiscard]] WorldPosition snapPositionToPixelGrid(const WorldPosition& position, const WorldSize& pixelStep) noexcept
{
    const float stepX = pixelStep.width > 0.0F ? pixelStep.width : 1.0F;
    const float stepY = pixelStep.height > 0.0F ? pixelStep.height : 1.0F;

    return {
        std::round(position.x / stepX) * stepX,
        std::round(position.y / stepY) * stepY};
}

} // namespace

std::string buildDebugOverlayString(
    const OverworldDebugSnapshot& debugSnapshot,
    const DebugOverlayRenderMetrics& renderMetrics,
    const int displayedFramesPerSecond)
{
    std::ostringstream overlayStream;
    overlayStream
        << "FPS: " << displayedFramesPerSecond << '\n'
        << "Retained chunks: " << debugSnapshot.retainedChunkCount << '\n'
        << "Retained objects: " << debugSnapshot.retainedGeneratedContentCount << '\n'
        << "Visible tiles: " << debugSnapshot.visibleTileCount << '\n'
        << "Visible content: " << debugSnapshot.visibleGeneratedContentCount << '\n'
        << "Front occluders: " << renderMetrics.frontOccluderCount << '\n'
        << "Occlusion candidates: " << renderMetrics.overlapQualifiedOcclusionCandidateCount << '\n'
        << "Grid vertices: " << renderMetrics.gridVertexCount << '\n'
        << "Terrain vertices: " << renderMetrics.terrainVertexCount << '\n'
        << "Coordinates: (" << debugSnapshot.playerTileCoordinates.x << ", " << debugSnapshot.playerTileCoordinates.y
        << ")\n"
        << "Zoom: " << debugSnapshot.zoomPercent << '%';
    return overlayStream.str();
}

ViewFrame snapViewFrameToPixelGrid(const ViewFrame& frame, const WorldSize& viewportSizeInPixels) noexcept
{
    const WorldSize pixelStep{
        viewportSizeInPixels.width > 0.0F ? frame.size.width / viewportSizeInPixels.width : 1.0F,
        viewportSizeInPixels.height > 0.0F ? frame.size.height / viewportSizeInPixels.height : 1.0F};
    return {snapPositionToPixelGrid(frame.center, pixelStep), frame.size};
}

std::vector<std::size_t> collectFrontGeneratedContentIndices(
    const std::vector<OverworldRenderQueueEntry>& renderQueue)
{
    std::vector<std::size_t> generatedContentIndices;
    bool passedPlayerMarker = false;

    for (const OverworldRenderQueueEntry& entry : renderQueue)
    {
        if (entry.kind == OverworldRenderQueueEntryKind::PlayerMarker)
        {
            passedPlayerMarker = true;
            continue;
        }

        if (passedPlayerMarker)
        {
            generatedContentIndices.push_back(entry.sourceIndex);
        }
    }

    return generatedContentIndices;
}

std::vector<std::size_t> collectOverlapQualifiedFrontGeneratedContentIndices(
    const std::vector<std::size_t>& frontGeneratedContentIndices,
    const std::vector<OverworldRenderContent>& generatedContent,
    const OverworldRenderMarker& playerMarker)
{
    std::vector<std::size_t> overlapQualifiedIndices;
    const WorldRectangle playerBounds = getWorldRectangle(playerMarker);

    for (const std::size_t index : frontGeneratedContentIndices)
    {
        if (index >= generatedContent.size())
        {
            continue;
        }

        if (doWorldRectanglesIntersect(playerBounds, getWorldRectangle(generatedContent[index])))
        {
            overlapQualifiedIndices.push_back(index);
        }
    }

    return overlapQualifiedIndices;
}

} // namespace detail
} // namespace rpg
