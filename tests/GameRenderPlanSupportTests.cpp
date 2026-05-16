/**
 * @file GameRenderPlanSupportTests.cpp
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

namespace
{

[[nodiscard]] bool verifyRenderQueueAssemblyAndOrdering()
{
    const rpg::OverworldRenderContent backgroundVegetation{
        11,
        rpg::ContentType::Shrub,
        "bush_small_1",
        {3, 6},
        {32.0F, 32.0F},
        {8.0F, 24.0F},
        {56.0F, 104.0F},
        {18},
        104.0F};
    const rpg::OverworldRenderContent frontVegetation{
        13,
        rpg::ContentType::Tree,
        "tree_small_1",
        {5, 10},
        {48.0F, 48.0F},
        {24.0F, 40.0F},
        {88.0F, 152.0F},
        {20},
        152.0F};
    const rpg::OverworldRenderMarker playerMarker{
        {48.0F, 48.0F},
        {24.0F, 32.0F},
        {72.0F, 136.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Down,
        1,
        136.0F};

    const rpg::OverworldRenderSnapshot renderSnapshot{
        {{0.0F, 0.0F}, {320.0F, 180.0F}},
        {},
        {},
        {backgroundVegetation, frontVegetation},
        {playerMarker}};

    const std::vector<rpg::detail::OverworldRenderQueueEntry> renderQueue =
        rpg::detail::buildOverworldRenderQueue(renderSnapshot);

    return renderQueue.size() == 3U
        && renderQueue[0].kind == rpg::detail::OverworldRenderQueueEntryKind::GeneratedContent
        && renderQueue[0].sourceIndex == 0U
        && renderQueue[1].kind == rpg::detail::OverworldRenderQueueEntryKind::PlayerMarker
        && renderQueue[1].sourceIndex == 0U
        && renderQueue[2].kind == rpg::detail::OverworldRenderQueueEntryKind::GeneratedContent
        && renderQueue[2].sourceIndex == 1U;
}

[[nodiscard]] bool verifyPlayerMarkerLookup()
{
    const rpg::OverworldRenderMarker playerMarker{
        {48.0F, 48.0F},
        {24.0F, 32.0F},
        {72.0F, 136.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Down,
        1,
        136.0F};
    const rpg::OverworldRenderMarker nonPlayerMarker{
        {48.0F, 48.0F},
        {24.0F, 32.0F},
        {96.0F, 136.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Left,
        1,
        136.0F};

    const rpg::OverworldRenderSnapshot snapshotWithPlayer{
        {{0.0F, 0.0F}, {320.0F, 180.0F}},
        {},
        {},
        {},
        {nonPlayerMarker, playerMarker}};
    const rpg::OverworldRenderSnapshot snapshotWithoutMarkers{
        {{0.0F, 0.0F}, {320.0F, 180.0F}},
        {},
        {},
        {},
        {}};

    return rpg::detail::findPlayerMarkerIndex(snapshotWithPlayer) == std::optional<std::size_t>{0U}
        && !rpg::detail::findPlayerMarkerIndex(snapshotWithoutMarkers).has_value();
}

[[nodiscard]] bool verifyRenderPlanQualifiesOccludersAndMetrics()
{
    const rpg::OverworldRenderContent backgroundVegetation{
        11,
        rpg::ContentType::Shrub,
        "bush_small_1",
        {3, 6},
        {32.0F, 32.0F},
        {8.0F, 24.0F},
        {56.0F, 104.0F},
        {18},
        104.0F};
    const rpg::OverworldRenderContent overlappingFrontVegetation{
        13,
        rpg::ContentType::Tree,
        "tree_small_1",
        {5, 10},
        {48.0F, 48.0F},
        {24.0F, 40.0F},
        {88.0F, 152.0F},
        {20},
        152.0F};
    const rpg::OverworldRenderContent nonOverlappingFrontVegetation{
        14,
        rpg::ContentType::Tree,
        "tree_small_1",
        {18, 10},
        {48.0F, 48.0F},
        {24.0F, 40.0F},
        {296.0F, 152.0F},
        {21},
        152.0F};
    const rpg::OverworldRenderMarker playerMarker{
        {48.0F, 48.0F},
        {24.0F, 32.0F},
        {72.0F, 136.0F},
        rpg::OverworldRenderMarkerAppearance::Player,
        rpg::PlayerFacingDirection::Down,
        1,
        136.0F};

    const rpg::OverworldRenderSnapshot renderSnapshot{
        {{0.0F, 0.0F}, {320.0F, 180.0F}},
        {},
        {},
        {backgroundVegetation, overlappingFrontVegetation, nonOverlappingFrontVegetation},
        {playerMarker}};

    const rpg::detail::OverworldRenderPlan renderPlan =
        rpg::detail::buildOverworldRenderPlan(renderSnapshot, 1536U, 36U);

    return renderPlan.playerMarkerIndex == std::optional<std::size_t>{0U}
        && renderPlan.frontOccluderIndices == std::vector<std::size_t>({1U, 2U})
        && renderPlan.overlapQualifiedOcclusionCandidateIndices == std::vector<std::size_t>({1U})
        && renderPlan.debugOverlayRenderMetrics.frontOccluderCount == 2U
        && renderPlan.debugOverlayRenderMetrics.overlapQualifiedOcclusionCandidateCount == 1U
        && renderPlan.debugOverlayRenderMetrics.terrainVertexCount == 1536U
        && renderPlan.debugOverlayRenderMetrics.gridVertexCount == 36U;
}

} // namespace

int main()
{
    if (!verifyRenderQueueAssemblyAndOrdering())
    {
        return 1;
    }

    if (!verifyPlayerMarkerLookup())
    {
        return 1;
    }

    if (!verifyRenderPlanQualifiesOccludersAndMetrics())
    {
        return 1;
    }

    return 0;
}
