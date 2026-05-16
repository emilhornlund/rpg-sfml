/**
 * @file WorldTerrainGeneratorTests.cpp
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

#include "BiomeSampler.hpp"
#include "GameAssetSupport.hpp"
#include "RoadNetworkSupport.hpp"
#include "RoadStampSupport.hpp"
#include "WorldContent.hpp"
#include "WorldTerrainGenerator.hpp"

#include <main/World.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

namespace
{

constexpr float kFloatTolerance = 0.001F;

[[nodiscard]] std::size_t toIndex(const rpg::TileCoordinates& coordinates, const int widthInTiles) noexcept
{
    return static_cast<std::size_t>(coordinates.y * widthInTiles + coordinates.x);
}

[[nodiscard]] bool areClose(const float lhs, const float rhs) noexcept
{
    return std::fabs(lhs - rhs) < kFloatTolerance;
}

[[nodiscard]] bool areEqual(const rpg::ChunkMetadata& lhs, const rpg::ChunkMetadata& rhs) noexcept
{
    if (lhs.chunkCoordinates.x != rhs.chunkCoordinates.x
        || lhs.chunkCoordinates.y != rhs.chunkCoordinates.y
        || lhs.biomeSummary.dominantTileType != rhs.biomeSummary.dominantTileType
        || lhs.biomeSummary.waterTileCount != rhs.biomeSummary.waterTileCount
        || lhs.biomeSummary.sandTileCount != rhs.biomeSummary.sandTileCount
        || lhs.biomeSummary.grassTileCount != rhs.biomeSummary.grassTileCount
        || lhs.biomeSummary.forestTileCount != rhs.biomeSummary.forestTileCount
        || lhs.traversabilitySummary.traversableTileCount != rhs.traversabilitySummary.traversableTileCount
        || lhs.traversabilitySummary.blockedTileCount != rhs.traversabilitySummary.blockedTileCount)
    {
        return false;
    }

    return true;
}

[[nodiscard]] bool areEqual(const rpg::ContentInstance& lhs, const rpg::ContentInstance& rhs) noexcept
{
    return lhs.id == rhs.id
        && lhs.type == rhs.type
        && lhs.prototypeId == rhs.prototypeId
        && lhs.anchorTile.x == rhs.anchorTile.x
        && lhs.anchorTile.y == rhs.anchorTile.y
        && areClose(lhs.position.x, rhs.position.x)
        && areClose(lhs.position.y, rhs.position.y)
        && areClose(lhs.footprint.offset.x, rhs.footprint.offset.x)
        && areClose(lhs.footprint.offset.y, rhs.footprint.offset.y)
        && areClose(lhs.footprint.size.width, rhs.footprint.size.width)
        && areClose(lhs.footprint.size.height, rhs.footprint.size.height)
        && lhs.appearanceId.value == rhs.appearanceId.value
        && areClose(lhs.sortKeyY, rhs.sortKeyY);
}

[[nodiscard]] bool areEqual(
    const std::vector<rpg::ContentInstance>& lhs,
    const std::vector<rpg::ContentInstance>& rhs) noexcept
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    for (std::size_t index = 0; index < lhs.size(); ++index)
    {
        if (!areEqual(lhs[index], rhs[index]))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool areEqual(const rpg::ChunkContent& lhs, const rpg::ChunkContent& rhs) noexcept
{
    return lhs.chunkCoordinates.x == rhs.chunkCoordinates.x
        && lhs.chunkCoordinates.y == rhs.chunkCoordinates.y
        && areEqual(lhs.instances, rhs.instances);
}

[[nodiscard]] bool areEqual(const rpg::detail::RoadNode& lhs, const rpg::detail::RoadNode& rhs) noexcept
{
    return lhs.coordinates.x == rhs.coordinates.x
        && lhs.coordinates.y == rhs.coordinates.y
        && lhs.anchorKind == rhs.anchorKind
        && lhs.topology == rhs.topology
        && lhs.stampMetadata.footprintWidth == rhs.stampMetadata.footprintWidth
        && lhs.stampMetadata.shoulderWidth == rhs.stampMetadata.shoulderWidth;
}

[[nodiscard]] bool areEqual(const rpg::detail::RoadSegment& lhs, const rpg::detail::RoadSegment& rhs) noexcept
{
    if (lhs.fromNodeIndex != rhs.fromNodeIndex
        || lhs.toNodeIndex != rhs.toNodeIndex
        || lhs.polyline.size() != rhs.polyline.size()
        || lhs.stampMetadata.roadClass != rhs.stampMetadata.roadClass
        || lhs.stampMetadata.carriageWidth != rhs.stampMetadata.carriageWidth
        || lhs.stampMetadata.shoulderWidth != rhs.stampMetadata.shoulderWidth
        || lhs.stampMetadata.stampBends != rhs.stampMetadata.stampBends)
    {
        return false;
    }

    for (std::size_t pointIndex = 0U; pointIndex < lhs.polyline.size(); ++pointIndex)
    {
        if (lhs.polyline[pointIndex].x != rhs.polyline[pointIndex].x
            || lhs.polyline[pointIndex].y != rhs.polyline[pointIndex].y)
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool areEqual(const rpg::detail::RoadNetwork& lhs, const rpg::detail::RoadNetwork& rhs) noexcept
{
    if (lhs.spawnTile.x != rhs.spawnTile.x
        || lhs.spawnTile.y != rhs.spawnTile.y
        || lhs.seed != rhs.seed
        || lhs.nodes.size() != rhs.nodes.size()
        || lhs.segments.size() != rhs.segments.size())
    {
        return false;
    }

    for (std::size_t nodeIndex = 0U; nodeIndex < lhs.nodes.size(); ++nodeIndex)
    {
        if (!areEqual(lhs.nodes[nodeIndex], rhs.nodes[nodeIndex]))
        {
            return false;
        }
    }

    for (std::size_t segmentIndex = 0U; segmentIndex < lhs.segments.size(); ++segmentIndex)
    {
        if (!areEqual(lhs.segments[segmentIndex], rhs.segments[segmentIndex]))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool containsVisibleContent(
    const std::vector<rpg::VisibleWorldContent>& visibleContent,
    const rpg::ContentInstance& instance) noexcept
{
    for (const rpg::VisibleWorldContent& visibleInstance : visibleContent)
    {
        if (areEqual(visibleInstance.instance, instance))
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool containsVisibleRoadOverlay(
    const std::vector<rpg::VisibleWorldRoadOverlay>& visibleRoadOverlays,
    const rpg::TileCoordinates& coordinates) noexcept
{
    for (const rpg::VisibleWorldRoadOverlay& roadOverlay : visibleRoadOverlays)
    {
        if (roadOverlay.coordinates.x == coordinates.x && roadOverlay.coordinates.y == coordinates.y)
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] bool anyGroundContentOccupiesRoad(
    const rpg::World& world,
    const std::vector<rpg::VisibleWorldContent>& visibleContent) noexcept
{
    for (const rpg::VisibleWorldContent& visibleInstance : visibleContent)
    {
        if (visibleInstance.instance.type == rpg::ContentType::Shrub
            && world.hasRoadOverlay(visibleInstance.instance.anchorTile))
        {
            return true;
        }
    }

    return false;
}

[[nodiscard]] std::size_t countRoadNodesWithAnchor(
    const rpg::detail::RoadNetwork& network,
    const rpg::detail::RoadNodeAnchorKind anchorKind) noexcept
{
    return static_cast<std::size_t>(std::count_if(
        network.nodes.begin(),
        network.nodes.end(),
        [anchorKind](const rpg::detail::RoadNode& node)
        {
            return node.anchorKind == anchorKind;
        }));
}

[[nodiscard]] std::size_t getRoadNodeConnectionCount(
    const rpg::detail::RoadNetwork& network,
    const std::size_t nodeIndex) noexcept
{
    std::size_t connectionCount = 0U;

    for (const rpg::detail::RoadSegment& segment : network.segments)
    {
        if (segment.fromNodeIndex == nodeIndex || segment.toNodeIndex == nodeIndex)
        {
            ++connectionCount;
        }
    }

    return connectionCount;
}

[[nodiscard]] bool doesRoadNetworkContainLoop(
    const rpg::detail::RoadNetwork& network) noexcept
{
    if (network.nodes.empty())
    {
        return false;
    }

    std::vector<std::vector<std::size_t>> adjacency(network.nodes.size());

    for (const rpg::detail::RoadSegment& segment : network.segments)
    {
        adjacency[segment.fromNodeIndex].push_back(segment.toNodeIndex);
        adjacency[segment.toNodeIndex].push_back(segment.fromNodeIndex);
    }

    std::vector<bool> visited(network.nodes.size(), false);
    const auto visit = [&adjacency, &visited](const auto& self, const std::size_t nodeIndex, const std::size_t parentIndex) -> bool
    {
        visited[nodeIndex] = true;

        for (const std::size_t neighborIndex : adjacency[nodeIndex])
        {
            if (!visited[neighborIndex])
            {
                if (self(self, neighborIndex, nodeIndex))
                {
                    return true;
                }
            }
            else if (neighborIndex != parentIndex)
            {
                return true;
            }
        }

        return false;
    };

    return visit(visit, 0U, network.nodes.size());
}

[[nodiscard]] bool isRoadNetworkFullyConnected(
    const rpg::detail::RoadNetwork& network) noexcept
{
    if (network.nodes.empty())
    {
        return false;
    }

    std::vector<bool> visited(network.nodes.size(), false);
    std::vector<std::size_t> stack{0U};
    visited[0U] = true;

    while (!stack.empty())
    {
        const std::size_t nodeIndex = stack.back();
        stack.pop_back();

        for (const rpg::detail::RoadSegment& segment : network.segments)
        {
            std::optional<std::size_t> nextNodeIndex;

            if (segment.fromNodeIndex == nodeIndex && !visited[segment.toNodeIndex])
            {
                nextNodeIndex = segment.toNodeIndex;
            }
            else if (segment.toNodeIndex == nodeIndex && !visited[segment.fromNodeIndex])
            {
                nextNodeIndex = segment.fromNodeIndex;
            }

            if (nextNodeIndex.has_value())
            {
                visited[*nextNodeIndex] = true;
                stack.push_back(*nextNodeIndex);
            }
        }
    }

    return std::all_of(visited.begin(), visited.end(), [](const bool value)
    {
        return value;
    });
}

[[nodiscard]] bool isInBounds(const rpg::TileCoordinates& coordinates, const rpg::WorldConfig& config) noexcept
{
    return coordinates.x >= 0
        && coordinates.x < config.widthInTiles
        && coordinates.y >= 0
        && coordinates.y < config.heightInTiles;
}

[[nodiscard]] bool hasAdjacentTileType(
    const std::vector<rpg::TileType>& tiles,
    const rpg::TileCoordinates& coordinates,
    const rpg::WorldConfig& config,
    const rpg::TileType tileType) noexcept
{
    constexpr std::array<rpg::TileCoordinates, 4> kOffsets = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    for (const rpg::TileCoordinates& offset : kOffsets)
    {
        const rpg::TileCoordinates candidate{coordinates.x + offset.x, coordinates.y + offset.y};

        if (isInBounds(candidate, config)
            && tiles[toIndex(candidate, config.widthInTiles)] == tileType)
        {
            return true;
        }
    }

    return false;
}

template <std::size_t kHeight>
[[nodiscard]] bool containsRoadOverlayPattern(
    const std::uint32_t seed,
    const std::array<std::string_view, kHeight>& pattern)
{
    constexpr rpg::TileCoordinates kSpawnTile{0, 0};
    constexpr rpg::TileType kSurface = rpg::TileType::Grass;
    constexpr int kRoadSearchLimit = 52;
    const rpg::detail::RoadNetwork network = rpg::detail::buildRoadNetwork(kSpawnTile, seed);
    const int patternWidth = static_cast<int>(pattern[0].size());

    for (const std::string_view row : pattern)
    {
        if (static_cast<int>(row.size()) != patternWidth)
        {
            return false;
        }
    }

    for (int originY = -kRoadSearchLimit; originY <= kRoadSearchLimit; ++originY)
    {
        for (int originX = -kRoadSearchLimit; originX <= kRoadSearchLimit; ++originX)
        {
            bool matchesPattern = true;

            for (std::size_t rowIndex = 0U; rowIndex < pattern.size() && matchesPattern; ++rowIndex)
            {
                for (int columnIndex = 0; columnIndex < patternWidth; ++columnIndex)
                {
                    const bool expectedRoad = pattern[rowIndex][static_cast<std::size_t>(columnIndex)] == '#';
                    const bool hasRoad = rpg::detail::hasPublishedStampedRoadOverlayAt(
                        {originX + columnIndex, originY + static_cast<int>(rowIndex)},
                        kSurface,
                        network,
                        [](const rpg::TileCoordinates&)
                        {
                            return kSurface;
                        });

                    if (hasRoad != expectedRoad)
                    {
                        matchesPattern = false;
                        break;
                    }
                }
            }

            if (matchesPattern)
            {
                return true;
            }
        }
    }

    return false;
}

struct CardinalNeighborMask
{
    bool north = false;
    bool east = false;
    bool south = false;
    bool west = false;
};

[[nodiscard]] CardinalNeighborMask getMatchingCardinalNeighborMask(
    const rpg::World& world,
    const rpg::TileCoordinates& coordinates)
{
    const rpg::TileType tileType = world.getTileType(coordinates);

    return {
        world.getTileType({coordinates.x, coordinates.y - 1}) == tileType,
        world.getTileType({coordinates.x + 1, coordinates.y}) == tileType,
        world.getTileType({coordinates.x, coordinates.y + 1}) == tileType,
        world.getTileType({coordinates.x - 1, coordinates.y}) == tileType,
    };
}

[[nodiscard]] bool hasUnsupportedTerrainShape(
    const rpg::World& world,
    const rpg::TileCoordinates& coordinates)
{
    const CardinalNeighborMask mask = getMatchingCardinalNeighborMask(world, coordinates);
    const int cardinalNeighborCount =
        static_cast<int>(mask.north) + static_cast<int>(mask.east) + static_cast<int>(mask.south) + static_cast<int>(mask.west);

    return cardinalNeighborCount <= 1;
}

[[nodiscard]] bool verifyDeterministicGeneration()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::detail::TerrainGenerator terrainGenerator{config};
    const rpg::detail::GeneratedWorldData firstWorld = terrainGenerator.generateWorld();
    const rpg::detail::GeneratedWorldData secondWorld = terrainGenerator.generateWorld();

    return firstWorld.spawnTile.x == secondWorld.spawnTile.x
        && firstWorld.spawnTile.y == secondWorld.spawnTile.y
        && firstWorld.tiles == secondWorld.tiles;
}

[[nodiscard]] bool verifyDeterministicBiomeSampling()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::detail::BiomeSampler firstSampler{config};
    const rpg::detail::BiomeSampler secondSampler{config};
    const std::array<rpg::TileCoordinates, 5> sampleCoordinates = {{
        {-24, -12},
        {-1, 0},
        {0, 0},
        {16, 8},
        {72, 40},
    }};

    for (const rpg::TileCoordinates& coordinates : sampleCoordinates)
    {
        if (firstSampler.sampleTileType(coordinates.x, coordinates.y)
            != secondSampler.sampleTileType(coordinates.x, coordinates.y))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool verifyDeterministicChunkGeneration()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::detail::TerrainGenerator terrainGenerator{config};
    const rpg::detail::GeneratedChunkData firstChunk = terrainGenerator.generateChunk(1, -2);
    const rpg::detail::GeneratedChunkData secondChunk = terrainGenerator.generateChunk(1, -2);

    return firstChunk.chunkX == secondChunk.chunkX
        && firstChunk.chunkY == secondChunk.chunkY
        && firstChunk.tiles == secondChunk.tiles
        && areEqual(firstChunk.metadata, secondChunk.metadata);
}

[[nodiscard]] bool verifyGroundContentAvoidsRoadCoveredTiles()
{
    rpg::World world;
    const rpg::WorldPosition spawnPosition = world.getSpawnPosition();
    const rpg::ViewFrame frame{spawnPosition, {320.0F, 180.0F}};
    world.updateRetentionWindow(frame);
    const std::vector<rpg::VisibleWorldContent> visibleContent = world.getVisibleContent(frame);
    return !anyGroundContentOccupiesRoad(world, visibleContent);
}

[[nodiscard]] bool verifyVisibleRoadOverlaysPreserveTopologyBackedEndpoints()
{
    rpg::World world;
    const rpg::detail::RoadNetwork network =
        rpg::detail::buildRoadNetwork(world.getSpawnTile(), world.getGenerationSeed());
    const rpg::TileCoordinates endpoint = network.nodes.back().coordinates;
    const rpg::ViewFrame frame{
        world.getTileCenter(endpoint),
        {world.getTileSize() * 8.0F, world.getTileSize() * 8.0F}};
    const std::vector<rpg::VisibleWorldRoadOverlay> visibleRoadOverlays = world.getVisibleRoadOverlays(frame);

    return world.hasRoadOverlay(endpoint)
        && containsVisibleRoadOverlay(visibleRoadOverlays, endpoint)
        && !world.hasRoadOverlay({endpoint.x, endpoint.y - 2});
}

[[nodiscard]] bool verifyRoadNetworkGenerationIsDeterministic()
{
    constexpr std::uint32_t kSeed = 0x00C0FFEEU;
    constexpr rpg::TileCoordinates kSpawnTile{0, 0};
    const rpg::detail::RoadNetwork firstNetwork = rpg::detail::buildRoadNetwork(kSpawnTile, kSeed);
    const rpg::detail::RoadNetwork secondNetwork = rpg::detail::buildRoadNetwork(kSpawnTile, kSeed);
    return areEqual(firstNetwork, secondNetwork);
}

[[nodiscard]] bool verifyRoadNetworkConnectsSpawnAndSupportsDeterministicLayouts()
{
    constexpr rpg::TileCoordinates kSpawnTile{0, 0};
    constexpr std::array<std::uint32_t, 3> kSeeds = {{
        0x00C0FFEEU,
        0x12345678U,
        0x13572468U,
    }};

    bool foundLoop = false;
    bool foundBranch = false;
    bool foundNonLoop = false;

    for (const std::uint32_t seed : kSeeds)
    {
        const rpg::detail::RoadNetwork network = rpg::detail::buildRoadNetwork(kSpawnTile, seed);

        if (network.nodes.empty()
            || network.nodes.front().anchorKind != rpg::detail::RoadNodeAnchorKind::Spawn
            || network.nodes.front().coordinates.x != kSpawnTile.x
            || network.nodes.front().coordinates.y != kSpawnTile.y
            || countRoadNodesWithAnchor(network, rpg::detail::RoadNodeAnchorKind::Destination) < 4U
            || getRoadNodeConnectionCount(network, 0U) == 0U
            || !isRoadNetworkFullyConnected(network))
        {
            return false;
        }

        foundBranch = foundBranch || std::any_of(
            network.nodes.begin(),
            network.nodes.end(),
            [&network](const rpg::detail::RoadNode& node)
            {
                const std::size_t nodeIndex = static_cast<std::size_t>(&node - network.nodes.data());
                return (node.topology == rpg::detail::RoadNodeTopology::Tee
                        || node.topology == rpg::detail::RoadNodeTopology::Cross
                        || node.topology == rpg::detail::RoadNodeTopology::Plaza)
                    && getRoadNodeConnectionCount(network, nodeIndex) >= 3U;
            });
        foundLoop = foundLoop || doesRoadNetworkContainLoop(network);
        foundNonLoop = foundNonLoop || !doesRoadNetworkContainLoop(network);
    }

    return foundBranch && foundLoop && foundNonLoop;
}

[[nodiscard]] bool verifyRoadNetworkPublishesDeterministicStampMetadata()
{
    constexpr std::uint32_t kSeed = 0x00C0FFEEU;
    constexpr rpg::TileCoordinates kSpawnTile{0, 0};
    const rpg::detail::RoadNetwork firstNetwork = rpg::detail::buildRoadNetwork(kSpawnTile, kSeed);
    const rpg::detail::RoadNetwork secondNetwork = rpg::detail::buildRoadNetwork(kSpawnTile, kSeed);

    if (!areEqual(firstNetwork, secondNetwork))
    {
        return false;
    }

    const bool foundMainShoulder = std::any_of(
        firstNetwork.segments.begin(),
        firstNetwork.segments.end(),
        [](const rpg::detail::RoadSegment& segment)
        {
            return segment.stampMetadata.roadClass == rpg::detail::RoadSegmentClass::MainRoad
                && segment.stampMetadata.shoulderWidth > 0;
        });
    const bool foundRoadWithoutShoulder = std::any_of(
        firstNetwork.segments.begin(),
        firstNetwork.segments.end(),
        [](const rpg::detail::RoadSegment& segment)
        {
            return segment.stampMetadata.roadClass == rpg::detail::RoadSegmentClass::Road
                && segment.stampMetadata.shoulderWidth == 0;
        });
    const bool foundTrailWithoutShoulder = std::any_of(
        firstNetwork.segments.begin(),
        firstNetwork.segments.end(),
        [](const rpg::detail::RoadSegment& segment)
        {
            return segment.stampMetadata.roadClass == rpg::detail::RoadSegmentClass::Trail
                && segment.stampMetadata.shoulderWidth == 0;
        });
    const bool foundPlazaNode = std::any_of(
        firstNetwork.nodes.begin(),
        firstNetwork.nodes.end(),
        [](const rpg::detail::RoadNode& node)
        {
            return node.topology == rpg::detail::RoadNodeTopology::Plaza
                && node.stampMetadata.footprintWidth >= 3;
        });

    return !firstNetwork.nodes.empty()
        && firstNetwork.nodes.front().stampMetadata.shoulderWidth > 0
        && foundMainShoulder
        && foundRoadWithoutShoulder
        && foundTrailWithoutShoulder
        && foundPlazaNode;
}

[[nodiscard]] bool verifyStampedRoadSupportExpandsMainRoadsAndJunctions()
{
    constexpr rpg::TileType kSurface = rpg::TileType::Grass;
    constexpr rpg::TileCoordinates kSpawnTile{0, 0};
    const rpg::detail::RoadNetwork network = rpg::detail::buildRoadNetwork(kSpawnTile, 0x00C0FFEEU);
    const auto tileTypeLookup = [](const rpg::TileCoordinates&)
    {
        return kSurface;
    };
    const rpg::TileCoordinates spawn = network.nodes.front().coordinates;
    const rpg::TileCoordinates eastJunction = network.nodes[1].coordinates;
    const rpg::TileCoordinates eastDestination = network.nodes[3].coordinates;
    const rpg::detail::RoadStampedTile spawnStampedTile =
        rpg::detail::getRoadStampedTile(spawn, kSurface, network, tileTypeLookup);
    const rpg::detail::RoadStampedTile shoulderTile =
        rpg::detail::getRoadStampedTile({spawn.x - 1, spawn.y}, kSurface, network, tileTypeLookup);
    const rpg::detail::RoadStampedTile junctionStampedTile =
        rpg::detail::getRoadStampedTile(eastJunction, kSurface, network, tileTypeLookup);
    const rpg::detail::RoadStampedTile destinationStampedTile =
        rpg::detail::getRoadStampedTile(eastDestination, kSurface, network, tileTypeLookup);
    const int junctionCardinalConnections =
        static_cast<int>(junctionStampedTile.publishedNeighborOccupancy[0])
        + static_cast<int>(junctionStampedTile.publishedNeighborOccupancy[2])
        + static_cast<int>(junctionStampedTile.publishedNeighborOccupancy[4])
        + static_cast<int>(junctionStampedTile.publishedNeighborOccupancy[6]);

    return spawnStampedTile.isPublished
        && shoulderTile.stampedWidth > destinationStampedTile.stampedWidth
        && shoulderTile.isPublished
        && shoulderTile.isShoulder
        && junctionStampedTile.isPublished
        && junctionStampedTile.isIntersection
        && junctionCardinalConnections >= 3;
}

[[nodiscard]] bool verifyStampedRoadSupportKeepsBendsConnected()
{
    constexpr std::array<std::uint32_t, 3> kSeeds = {{
        0x00C0FFEEU,
        0x12345678U,
        0x13572468U,
    }};
    constexpr rpg::TileType kSurface = rpg::TileType::Grass;
    constexpr rpg::TileCoordinates kSpawnTile{0, 0};
    const auto tileTypeLookup = [](const rpg::TileCoordinates&)
    {
        return kSurface;
    };

    for (const std::uint32_t seed : kSeeds)
    {
        const rpg::detail::RoadNetwork network = rpg::detail::buildRoadNetwork(kSpawnTile, seed);

        for (const rpg::detail::RoadSegment& segment : network.segments)
        {
            if (segment.polyline.size() < 3U)
            {
                continue;
            }

            const rpg::TileCoordinates bendCoordinates = segment.polyline[1];
            const rpg::detail::RoadStampedTile bendTile =
                rpg::detail::getRoadStampedTile(bendCoordinates, kSurface, network, tileTypeLookup);
            const int cardinalConnections =
                static_cast<int>(bendTile.publishedNeighborOccupancy[0])
                + static_cast<int>(bendTile.publishedNeighborOccupancy[2])
                + static_cast<int>(bendTile.publishedNeighborOccupancy[4])
                + static_cast<int>(bendTile.publishedNeighborOccupancy[6]);

            return bendTile.isPublished
                && bendTile.isIntersection
                && cardinalConnections >= 2;
        }
    }

    return false;
}

[[nodiscard]] bool verifySharedRoadOverlaySupportAvoidsSingleTileNotches()
{
    constexpr std::array<std::uint32_t, 3> kSeeds = {{
        0x00C0FFEEU,
        0x12345678U,
        0x13572468U,
    }};
    constexpr std::array<std::string_view, 2> kDownwardNotch = {"###", ".#."};
    constexpr std::array<std::string_view, 2> kUpwardNotch = {".#.", "###"};
    constexpr std::array<std::string_view, 3> kRightFacingNotch = {"#.", "##", "#."};
    constexpr std::array<std::string_view, 3> kLeftFacingNotch = {".#", "##", ".#"};

    for (const std::uint32_t seed : kSeeds)
    {
        if (containsRoadOverlayPattern(seed, kDownwardNotch)
            || containsRoadOverlayPattern(seed, kUpwardNotch)
            || containsRoadOverlayPattern(seed, kRightFacingNotch)
            || containsRoadOverlayPattern(seed, kLeftFacingNotch))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool verifyOriginAnchoredSpawnSelection()
{
    const rpg::WorldConfig smallerWorldConfig{
        .seed = 0x13572468U,
        .widthInTiles = 36,
        .heightInTiles = 20,
        .tileSize = 20.0F};
    const rpg::WorldConfig largerWorldConfig{
        .seed = smallerWorldConfig.seed,
        .widthInTiles = 96,
        .heightInTiles = 72,
        .tileSize = smallerWorldConfig.tileSize};
    const rpg::World smallerWorld(smallerWorldConfig);
    const rpg::World largerWorld(largerWorldConfig);

    return smallerWorld.getSpawnTile().x == largerWorld.getSpawnTile().x
        && smallerWorld.getSpawnTile().y == largerWorld.getSpawnTile().y;
}

[[nodiscard]] bool verifySpawnValidity()
{
    rpg::World world({.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F});
    const rpg::TileCoordinates spawnTile = world.getSpawnTile();

    return world.isTraversable(spawnTile)
        && world.getTileCoordinates(world.getSpawnPosition()).x == spawnTile.x
        && world.getTileCoordinates(world.getSpawnPosition()).y == spawnTile.y;
}

[[nodiscard]] bool verifyCoastlinesAndInlandBiomes()
{
    const std::array<rpg::WorldConfig, 3> configs = {{
        {.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F},
        {.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F},
        {.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F},
    }};

    bool foundShorelineSand = false;
    bool foundInlandGrass = false;
    bool foundInlandForest = false;

    for (const rpg::WorldConfig& config : configs)
    {
        const rpg::detail::TerrainGenerator terrainGenerator{config};
        const rpg::detail::GeneratedWorldData worldData = terrainGenerator.generateWorld();

        for (int y = 1; y < config.heightInTiles - 1; ++y)
        {
            for (int x = 1; x < config.widthInTiles - 1; ++x)
            {
                const rpg::TileCoordinates coordinates{x, y};
                const rpg::TileType tileType = worldData.tiles[toIndex(coordinates, config.widthInTiles)];
                const bool adjacentToWater = hasAdjacentTileType(worldData.tiles, coordinates, config, rpg::TileType::Water);
                const bool isInland = x > 2
                    && x < config.widthInTiles - 3
                    && y > 2
                    && y < config.heightInTiles - 3
                    && !adjacentToWater;

                if (tileType == rpg::TileType::Sand && adjacentToWater)
                {
                    foundShorelineSand = true;
                }

                if (!isInland)
                {
                    continue;
                }

                if (tileType == rpg::TileType::Grass)
                {
                    foundInlandGrass = true;
                }
                else if (tileType == rpg::TileType::Forest)
                {
                    foundInlandForest = true;
                }
            }
        }
    }

    return foundShorelineSand && foundInlandGrass && foundInlandForest;
}

[[nodiscard]] bool verifyNegativeChunkCoordinateMapping()
{
    const rpg::TileCoordinates leftOfOrigin{
        rpg::detail::getChunkLocalCoordinate(-1),
        rpg::detail::getChunkLocalCoordinate(-17)};
    const rpg::TileCoordinates recoveredLeftOfOrigin = rpg::detail::getWorldTileCoordinates(-1, -2, leftOfOrigin);
    const rpg::TileCoordinates origin{
        rpg::detail::getChunkLocalCoordinate(0),
        rpg::detail::getChunkLocalCoordinate(16)};
    const rpg::TileCoordinates recoveredOrigin = rpg::detail::getWorldTileCoordinates(0, 1, origin);

    return rpg::detail::getChunkCoordinate(-1) == -1
        && rpg::detail::getChunkCoordinate(-16) == -1
        && rpg::detail::getChunkCoordinate(-17) == -2
        && rpg::detail::getChunkCoordinate(15) == 0
        && rpg::detail::getChunkCoordinate(16) == 1
        && leftOfOrigin.x == rpg::detail::getChunkSizeInTiles() - 1
        && leftOfOrigin.y == rpg::detail::getChunkSizeInTiles() - 1
        && recoveredLeftOfOrigin.x == -1
        && recoveredLeftOfOrigin.y == -17
        && origin.x == 0
        && origin.y == 0
        && recoveredOrigin.x == 0
        && recoveredOrigin.y == 16;
}

[[nodiscard]] bool verifyChunkGenerationExtendsBeyondInitialArea()
{
    const rpg::WorldConfig config{.seed = 0x2468ACE0U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::TileCoordinates farCoordinates{
        rpg::detail::getChunkSizeInTiles() * 8 + 1,
        rpg::detail::getChunkSizeInTiles() * 6 + 2};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();

    (void)world.getTileType(farCoordinates);
    const std::size_t generatedAfterFirstFarQuery = rpg::detail::getGeneratedChunkCount();

    if (generatedAfterFirstFarQuery <= generatedAfterConstruction)
    {
        return false;
    }

    (void)world.getTileType({farCoordinates.x + 1, farCoordinates.y + 1});
    (void)world.getTileType(farCoordinates);

    return rpg::detail::getGeneratedChunkCount() == generatedAfterFirstFarQuery;
}

[[nodiscard]] bool verifyVisibleRenderQueriesGenerateAndReuseWorldCache()
{
    const rpg::WorldConfig config{.seed = 0x0BADCAFEU, .widthInTiles = 48, .heightInTiles = 32, .tileSize = 24.0F};
    const rpg::TileCoordinates farTile{
        rpg::detail::getChunkSizeInTiles() * 10,
        rpg::detail::getChunkSizeInTiles() * 8};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();
    const rpg::ViewFrame frame{
        world.getTileCenter(farTile),
        {config.tileSize * 8.0F, config.tileSize * 6.0F}};

    const std::vector<rpg::VisibleWorldTile> firstVisibleTiles = world.getVisibleTiles(frame);
    const std::size_t generatedAfterFirstFrame = rpg::detail::getGeneratedChunkCount();

    if (firstVisibleTiles.empty() || generatedAfterFirstFrame <= generatedAfterConstruction)
    {
        return false;
    }

    (void)world.getVisibleTiles(frame);

    return rpg::detail::getGeneratedChunkCount() == generatedAfterFirstFrame;
}

[[nodiscard]] bool verifyMetadataQueriesGenerateAndReuseWorldCache()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::ChunkCoordinates farChunk{9, -7};
    const rpg::TileCoordinates tileInFarChunk{
        farChunk.x * rpg::detail::getChunkSizeInTiles() + 3,
        farChunk.y * rpg::detail::getChunkSizeInTiles() + 5};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();

    const rpg::ChunkMetadata firstMetadata = world.getChunkMetadata(farChunk);
    const std::size_t generatedAfterMetadataQuery = rpg::detail::getGeneratedChunkCount();

    if (generatedAfterMetadataQuery <= generatedAfterConstruction
        || firstMetadata.chunkCoordinates.x != farChunk.x
        || firstMetadata.chunkCoordinates.y != farChunk.y)
    {
        return false;
    }

    (void)world.getTileType(tileInFarChunk);
    const rpg::ChunkMetadata secondMetadata = world.getChunkMetadata(tileInFarChunk);

    return rpg::detail::getGeneratedChunkCount() == generatedAfterMetadataQuery
        && areEqual(firstMetadata, secondMetadata);
}

[[nodiscard]] bool verifyBorrowedMetadataQueriesExposeRetainedChunkData()
{
    const rpg::WorldConfig config{.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F};
    const rpg::ChunkCoordinates farChunk{9, -7};
    const rpg::TileCoordinates tileInFarChunk{
        farChunk.x * rpg::detail::getChunkSizeInTiles() + 3,
        farChunk.y * rpg::detail::getChunkSizeInTiles() + 5};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();

    const rpg::ChunkMetadata& firstMetadata = world.getChunkMetadataRef(farChunk);
    const std::size_t generatedAfterMetadataQuery = rpg::detail::getGeneratedChunkCount();
    const rpg::ChunkMetadata& repeatedMetadata = world.getChunkMetadataRef(tileInFarChunk);
    const rpg::ChunkMetadata snapshotMetadata = world.getChunkMetadata(farChunk);

    return generatedAfterMetadataQuery > generatedAfterConstruction
        && rpg::detail::getGeneratedChunkCount() == generatedAfterMetadataQuery
        && &firstMetadata == &repeatedMetadata
        && areEqual(firstMetadata, repeatedMetadata)
        && areEqual(firstMetadata, snapshotMetadata);
}

[[nodiscard]] bool verifyContentQueriesGenerateAndReuseWorldCache()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::ChunkCoordinates farChunk{9, -7};
    const rpg::TileCoordinates tileInFarChunk{
        farChunk.x * rpg::detail::getChunkSizeInTiles() + 3,
        farChunk.y * rpg::detail::getChunkSizeInTiles() + 5};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();

    const rpg::ChunkContent firstContent = world.getChunkContent(farChunk);
    const std::size_t generatedAfterContentQuery = rpg::detail::getGeneratedChunkCount();

    if (generatedAfterContentQuery <= generatedAfterConstruction)
    {
        return false;
    }

    (void)world.getTileType(tileInFarChunk);
    const rpg::ChunkContent secondContent = world.getChunkContent(tileInFarChunk);

    return rpg::detail::getGeneratedChunkCount() == generatedAfterContentQuery
        && areEqual(firstContent, secondContent);
}

[[nodiscard]] bool verifyBorrowedContentQueriesExposeRetainedChunkData()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::ChunkCoordinates farChunk{9, -7};
    const rpg::TileCoordinates tileInFarChunk{
        farChunk.x * rpg::detail::getChunkSizeInTiles() + 3,
        farChunk.y * rpg::detail::getChunkSizeInTiles() + 5};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();

    const rpg::ChunkContent& firstContent = world.getChunkContentRef(farChunk);
    const std::size_t generatedAfterContentQuery = rpg::detail::getGeneratedChunkCount();
    const rpg::ChunkContent& repeatedContent = world.getChunkContentRef(tileInFarChunk);
    const rpg::ChunkContent snapshotContent = world.getChunkContent(farChunk);

    return generatedAfterContentQuery > generatedAfterConstruction
        && rpg::detail::getGeneratedChunkCount() == generatedAfterContentQuery
        && &firstContent == &repeatedContent
        && areEqual(firstContent, repeatedContent)
        && areEqual(firstContent, snapshotContent);
}

[[nodiscard]] bool verifyMissingChunkLoadsReuseRetainedGenerationHelpers()
{
    const rpg::WorldConfig config{.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F};
    const rpg::ChunkCoordinates firstChunk{9, -7};
    const rpg::ChunkCoordinates secondChunk{10, -7};
    const rpg::TileCoordinates tileInFirstChunk{
        firstChunk.x * rpg::detail::getChunkSizeInTiles() + 3,
        firstChunk.y * rpg::detail::getChunkSizeInTiles() + 5};
    rpg::detail::resetGeneratedChunkCount();
    rpg::detail::resetTerrainGeneratorConstructionCount();
    rpg::detail::resetWorldContentConstructionCount();
    rpg::World world(config);
    const std::size_t generatedAfterConstruction = rpg::detail::getGeneratedChunkCount();
    const std::size_t terrainGeneratorConstructionsAfterConstruction =
        rpg::detail::getTerrainGeneratorConstructionCount();
    const std::size_t worldContentConstructionsAfterConstruction =
        rpg::detail::getWorldContentConstructionCount();

    if (terrainGeneratorConstructionsAfterConstruction != 1 || worldContentConstructionsAfterConstruction != 1)
    {
        return false;
    }

    const rpg::ChunkContent firstContent = world.getChunkContent(firstChunk);
    const std::size_t generatedAfterFirstChunk = rpg::detail::getGeneratedChunkCount();

    if (generatedAfterFirstChunk <= generatedAfterConstruction
        || rpg::detail::getTerrainGeneratorConstructionCount() != terrainGeneratorConstructionsAfterConstruction
        || rpg::detail::getWorldContentConstructionCount() != worldContentConstructionsAfterConstruction)
    {
        return false;
    }

    const rpg::ChunkContent secondContent = world.getChunkContent(secondChunk);
    const std::size_t generatedAfterSecondChunk = rpg::detail::getGeneratedChunkCount();

    if (generatedAfterSecondChunk <= generatedAfterFirstChunk
        || secondContent.chunkCoordinates.x != secondChunk.x
        || secondContent.chunkCoordinates.y != secondChunk.y
        || rpg::detail::getTerrainGeneratorConstructionCount() != terrainGeneratorConstructionsAfterConstruction
        || rpg::detail::getWorldContentConstructionCount() != worldContentConstructionsAfterConstruction)
    {
        return false;
    }

    (void)world.getTileType(tileInFirstChunk);
    const rpg::ChunkContent repeatedFirstContent = world.getChunkContent(firstChunk);

    return rpg::detail::getTerrainGeneratorConstructionCount() == terrainGeneratorConstructionsAfterConstruction
        && rpg::detail::getWorldContentConstructionCount() == worldContentConstructionsAfterConstruction
        && areEqual(firstContent, repeatedFirstContent);
}

[[nodiscard]] bool verifyChunkContentRecords()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    std::optional<rpg::ChunkContent> supportedChunk;

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkCoordinates chunkCoordinates{chunkX, chunkY};
            const rpg::ChunkMetadata metadata = world.getChunkMetadata(rpg::ChunkCoordinates{chunkX, chunkY});
            const rpg::ChunkContent content = world.getChunkContent(chunkCoordinates);

            if (!supportedChunk.has_value()
                && metadata.traversabilitySummary.traversableTileCount > metadata.traversabilitySummary.blockedTileCount)
            {
                supportedChunk = content;
            }
        }
    }

    if (!supportedChunk.has_value() || supportedChunk->instances.empty())
    {
        return false;
    }

    if (supportedChunk->chunkCoordinates.x != world.getChunkCoordinates(world.getTileCoordinates(supportedChunk->instances.front().position)).x
        || supportedChunk->chunkCoordinates.y != world.getChunkCoordinates(world.getTileCoordinates(supportedChunk->instances.front().position)).y)
    {
        return false;
    }

    for (const rpg::ContentInstance& instance : supportedChunk->instances)
    {
        const rpg::ChunkCoordinates owningChunk = world.getChunkCoordinates(world.getTileCoordinates(instance.position));

        if (instance.id == 0
            || owningChunk.x != supportedChunk->chunkCoordinates.x
            || owningChunk.y != supportedChunk->chunkCoordinates.y
            || instance.footprint.size.width <= 0.0F
            || instance.footprint.size.height <= 0.0F
            || instance.appearanceId.value == 0)
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool isWaterVegetationPrototypeId(const std::string& prototypeId) noexcept
{
    return prototypeId.starts_with("water_lily_") || prototypeId.starts_with("marsh_reeds_");
}

[[nodiscard]] bool isPlacementMode(
    const rpg::detail::VegetationPrototype& prototype,
    const rpg::detail::VegetationPlacementMode placementMode) noexcept
{
    return prototype.placementMode == placementMode;
}

[[nodiscard]] const char* getPlacementClassName(const rpg::TileType tileType) noexcept
{
    switch (tileType)
    {
    case rpg::TileType::Grass:
        return "grass";
    case rpg::TileType::Sand:
        return "sand";
    case rpg::TileType::Water:
        return "water";
    case rpg::TileType::Forest:
        return "forest";
    }

    return "";
}

[[nodiscard]] bool prototypeSupportsTileType(
    const rpg::detail::VegetationPrototype& prototype,
    const rpg::TileType tileType) noexcept
{
    const std::string_view placementClass = getPlacementClassName(tileType);
    const auto placeOnIt = std::find(prototype.placeOn.begin(), prototype.placeOn.end(), placementClass);

    if (placeOnIt == prototype.placeOn.end())
    {
        return false;
    }

    const auto biomeIt = std::find_if(
        prototype.biomes.begin(),
        prototype.biomes.end(),
        [placementClass](const auto& entry)
        {
            return entry.first == placementClass;
        });

    return biomeIt != prototype.biomes.end() && biomeIt->second > 0.0F;
}

[[nodiscard]] bool verifyVisibleContentQueries()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    std::optional<rpg::ContentInstance> sampleInstance;

    for (int chunkY = -12; chunkY <= 12 && !sampleInstance.has_value(); ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});

            if (!content.instances.empty())
            {
                sampleInstance = content.instances.front();
                break;
            }
        }
    }

    if (!sampleInstance.has_value())
    {
        return false;
    }

    const float frameSize = world.getTileSize() * 0.5F;
    const float overlap = world.getTileSize() * 0.25F;
    const float footprintLeft = sampleInstance->position.x + sampleInstance->footprint.offset.x;
    const float footprintRight = footprintLeft + sampleInstance->footprint.size.width;
    const float footprintVerticalCenter =
        sampleInstance->position.y + sampleInstance->footprint.offset.y + (sampleInstance->footprint.size.height * 0.5F);
    const rpg::ViewFrame intersectingFrame{
        {
            footprintRight + (frameSize * 0.5F) - overlap,
            footprintVerticalCenter,
        },
        {frameSize, frameSize}};
    const rpg::ViewFrame nonIntersectingFrame{
        {
            footprintRight + frameSize,
            footprintVerticalCenter,
        },
        {frameSize, frameSize}};

    const std::vector<rpg::VisibleWorldContent> intersectingContent = world.getVisibleContent(intersectingFrame);
    const std::vector<rpg::VisibleWorldContent> nonIntersectingContent = world.getVisibleContent(nonIntersectingFrame);

    return containsVisibleContent(intersectingContent, *sampleInstance)
        && !containsVisibleContent(nonIntersectingContent, *sampleInstance);
}

[[nodiscard]] bool verifyChunkRetentionWindowCanUnloadAndRegenerate()
{
    const rpg::WorldConfig config{.seed = 0x0BADCAFEU, .widthInTiles = 48, .heightInTiles = 32, .tileSize = 24.0F};
    rpg::detail::resetGeneratedChunkCount();
    rpg::World world(config);
    const rpg::ViewFrame firstFrame{
        {
            static_cast<float>(rpg::detail::getChunkSizeInTiles() * 10) * config.tileSize,
            static_cast<float>(rpg::detail::getChunkSizeInTiles() * 8) * config.tileSize,
        },
        {config.tileSize * 8.0F, config.tileSize * 6.0F}};

    world.updateRetentionWindow(firstFrame);
    const std::vector<rpg::VisibleWorldTile> firstVisibleTiles = world.getVisibleTiles(firstFrame);

    if (firstVisibleTiles.empty() || world.getRetainedChunkCount() == 0)
    {
        return false;
    }

    const rpg::ChunkCoordinates retainedChunk = world.getChunkCoordinates(firstVisibleTiles.front().coordinates);
    const rpg::ChunkMetadata retainedMetadata = world.getChunkMetadata(retainedChunk);
    const rpg::ChunkContent retainedContent = world.getChunkContent(retainedChunk);
    const rpg::ChunkMetadata& retainedMetadataRef = world.getChunkMetadataRef(retainedChunk);
    const rpg::ChunkContent& retainedContentRef = world.getChunkContentRef(retainedChunk);

    if (!areEqual(retainedMetadata, retainedMetadataRef) || !areEqual(retainedContent, retainedContentRef))
    {
        return false;
    }

    const std::size_t retainedChunkCount = world.getRetainedChunkCount();
    const std::size_t retainedContentCount = world.getRetainedGeneratedContentCount();
    const std::size_t generatedAfterFirstWindow = rpg::detail::getGeneratedChunkCount();
    const rpg::ViewFrame secondFrame{
        {
            static_cast<float>(-rpg::detail::getChunkSizeInTiles() * 10) * config.tileSize,
            static_cast<float>(-rpg::detail::getChunkSizeInTiles() * 8) * config.tileSize,
        },
        firstFrame.size};

    world.updateRetentionWindow(secondFrame);

    if (world.getRetainedChunkCount() >= retainedChunkCount
        || world.getRetainedGeneratedContentCount() >= retainedContentCount)
    {
        return false;
    }

    const std::size_t generatedBeforeReload = rpg::detail::getGeneratedChunkCount();
    const rpg::ChunkMetadata& reloadedMetadataRef = world.getChunkMetadataRef(retainedChunk);
    const rpg::ChunkContent& reloadedContentRef = world.getChunkContentRef(retainedChunk);

    return generatedBeforeReload == generatedAfterFirstWindow
        && rpg::detail::getGeneratedChunkCount() > generatedBeforeReload
        && areEqual(retainedMetadata, reloadedMetadataRef)
        && areEqual(retainedContent, reloadedContentRef);
}

[[nodiscard]] bool verifyVisibleQueriesStillWorkWithChunkUnloading()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    rpg::World world(config);
    std::optional<rpg::ContentInstance> sampleInstance;

    for (int chunkY = -12; chunkY <= 12 && !sampleInstance.has_value(); ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});

            if (!content.instances.empty())
            {
                sampleInstance = content.instances.front();
                break;
            }
        }
    }

    if (!sampleInstance.has_value())
    {
        return false;
    }

    const float frameSize = world.getTileSize() * 0.5F;
    const float overlap = world.getTileSize() * 0.25F;
    const float footprintLeft = sampleInstance->position.x + sampleInstance->footprint.offset.x;
    const float footprintRight = footprintLeft + sampleInstance->footprint.size.width;
    const float footprintVerticalCenter =
        sampleInstance->position.y + sampleInstance->footprint.offset.y + (sampleInstance->footprint.size.height * 0.5F);
    const rpg::ViewFrame frame{
        {
            footprintRight + (frameSize * 0.5F) - overlap,
            footprintVerticalCenter,
        },
        {frameSize, frameSize}};

    world.updateRetentionWindow(frame);
    const std::vector<rpg::VisibleWorldTile> visibleTiles = world.getVisibleTiles(frame);
    const std::vector<rpg::VisibleWorldContent> visibleContent = world.getVisibleContent(frame);

    return !visibleTiles.empty()
        && containsVisibleContent(visibleContent, *sampleInstance)
        && world.getRetainedChunkCount() > 0;
}

[[nodiscard]] bool verifyForestChunksProduceDenserVegetationThanGrassChunks()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    int forestChunkCount = 0;
    int grassChunkCount = 0;
    int forestTreeCount = 0;
    int grassTreeCount = 0;
    int forestInstanceCount = 0;
    int grassInstanceCount = 0;

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkCoordinates chunkCoordinates{chunkX, chunkY};
            const rpg::ChunkMetadata metadata = world.getChunkMetadata(chunkCoordinates);
            const rpg::ChunkContent content = world.getChunkContent(chunkCoordinates);
            const int treeCount = static_cast<int>(std::count_if(
                content.instances.begin(),
                content.instances.end(),
                [](const rpg::ContentInstance& instance)
                {
                    return instance.type == rpg::ContentType::Tree;
                }));

            if (metadata.biomeSummary.forestTileCount > metadata.biomeSummary.grassTileCount
                && metadata.biomeSummary.forestTileCount > 0)
            {
                ++forestChunkCount;
                forestTreeCount += treeCount;
                forestInstanceCount += static_cast<int>(content.instances.size());
                continue;
            }

            if (metadata.biomeSummary.grassTileCount > metadata.biomeSummary.forestTileCount
                && metadata.biomeSummary.grassTileCount > 0)
            {
                ++grassChunkCount;
                grassTreeCount += treeCount;
                grassInstanceCount += static_cast<int>(content.instances.size());
            }
        }
    }

    return forestChunkCount > 0
        && grassChunkCount > 0
        && forestTreeCount * grassChunkCount > grassTreeCount * forestChunkCount
        && forestInstanceCount * grassChunkCount > grassInstanceCount * forestChunkCount;
}

[[nodiscard]] bool verifyWaterVegetationPlacementIsDeterministic()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World firstWorld(config);
    const rpg::World secondWorld(config);
    std::optional<rpg::ChunkCoordinates> sampleChunk;

    for (int chunkY = -12; chunkY <= 12 && !sampleChunk.has_value(); ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkCoordinates chunkCoordinates{chunkX, chunkY};
            const rpg::ChunkContent content = firstWorld.getChunkContent(chunkCoordinates);
            const auto waterVegetationIt = std::find_if(
                content.instances.begin(),
                content.instances.end(),
                [](const rpg::ContentInstance& instance)
                {
                    return isWaterVegetationPrototypeId(instance.prototypeId);
                });

            if (waterVegetationIt != content.instances.end())
            {
                sampleChunk = chunkCoordinates;
                break;
            }
        }
    }

    if (!sampleChunk.has_value())
    {
        return false;
    }

    const rpg::ChunkContent firstContent = firstWorld.getChunkContent(*sampleChunk);
    const rpg::ChunkContent repeatedContent = firstWorld.getChunkContent(*sampleChunk);
    const rpg::ChunkContent secondContent = secondWorld.getChunkContent(*sampleChunk);

    if (!areEqual(firstContent, repeatedContent) || !areEqual(firstContent, secondContent))
    {
        return false;
    }

    bool foundWaterVegetation = false;

    for (const rpg::ContentInstance& instance : firstContent.instances)
    {
        if (!isWaterVegetationPrototypeId(instance.prototypeId))
        {
            continue;
        }

        foundWaterVegetation = true;

        if (firstWorld.getTileType(instance.anchorTile) != rpg::TileType::Water)
        {
            return false;
        }
    }

    return foundWaterVegetation;
}

[[nodiscard]] bool verifyVegetationPlacementRespectsPrototypeConstraints()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    const rpg::detail::VegetationTilesetMetadata metadata =
        rpg::detail::loadVegetationTilesetMetadata(std::filesystem::path(RPG_DEFAULT_ASSET_ROOT_PATH));
    bool foundWaterVegetation = false;

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});

            for (const rpg::ContentInstance& instance : content.instances)
            {
                const rpg::detail::VegetationPrototype& prototype = metadata.getPrototypeById(instance.prototypeId);
                const rpg::TileType anchorTileType = world.getTileType(instance.anchorTile);

                if (!prototypeSupportsTileType(prototype, anchorTileType))
                {
                    return false;
                }

                if (isWaterVegetationPrototypeId(instance.prototypeId))
                {
                    foundWaterVegetation = true;
                }
            }
        }
    }

    return foundWaterVegetation;
}

[[nodiscard]] bool verifyForestPlacementModeDensityHierarchy()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    const rpg::detail::VegetationTilesetMetadata metadata =
        rpg::detail::loadVegetationTilesetMetadata(std::filesystem::path(RPG_DEFAULT_ASSET_ROOT_PATH));
    int forestTreeSparseCount = 0;
    int forestGroundDenseCount = 0;
    int forestPropSparseCount = 0;

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});

            for (const rpg::ContentInstance& instance : content.instances)
            {
                if (world.getTileType(instance.anchorTile) != rpg::TileType::Forest)
                {
                    continue;
                }

                const rpg::detail::VegetationPrototype& prototype = metadata.getPrototypeById(instance.prototypeId);

                if (isPlacementMode(prototype, rpg::detail::VegetationPlacementMode::TreeSparse))
                {
                    ++forestTreeSparseCount;
                    continue;
                }

                if (isPlacementMode(prototype, rpg::detail::VegetationPlacementMode::GroundDense))
                {
                    ++forestGroundDenseCount;
                    continue;
                }

                if (isPlacementMode(prototype, rpg::detail::VegetationPlacementMode::PropSparse))
                {
                    ++forestPropSparseCount;
                }
            }
        }
    }

    return forestTreeSparseCount > forestGroundDenseCount
        && forestGroundDenseCount > forestPropSparseCount
        && forestPropSparseCount > 0;
}

[[nodiscard]] bool verifyLargeVegetationRemainsVisibleAcrossChunkBoundaries()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    std::optional<rpg::ContentInstance> spanningInstance;
    constexpr int kChunkSizeInTiles = rpg::detail::getChunkSizeInTiles();

    for (int chunkY = -12; chunkY <= 12 && !spanningInstance.has_value(); ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});
            const float chunkTop = static_cast<float>(chunkY * kChunkSizeInTiles) * world.getTileSize();

            for (const rpg::ContentInstance& instance : content.instances)
            {
                const float top = instance.position.y + instance.footprint.offset.y;

                if (instance.type == rpg::ContentType::Tree && top < chunkTop)
                {
                    spanningInstance = instance;
                    break;
                }
            }

            if (spanningInstance.has_value())
            {
                break;
            }
        }
    }

    if (!spanningInstance.has_value())
    {
        return false;
    }

    const float frameSize = world.getTileSize() * 0.5F;
    const float top = spanningInstance->position.y + spanningInstance->footprint.offset.y;
    const float frameCenterX = spanningInstance->position.x;
    const float frameCenterY = top + (frameSize * 0.5F);
    const std::vector<rpg::VisibleWorldContent> visibleContent = world.getVisibleContent(
        {{frameCenterX, frameCenterY}, {frameSize, frameSize}});

    return containsVisibleContent(visibleContent, *spanningInstance);
}

[[nodiscard]] bool verifyVegetationFootprintsMatchMetadataBounds()
{
    const rpg::WorldConfig config{.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F};
    const rpg::World world(config);
    const rpg::detail::VegetationTilesetMetadata metadata =
        rpg::detail::loadVegetationTilesetMetadata(std::filesystem::path(RPG_DEFAULT_ASSET_ROOT_PATH));

    for (int chunkY = -12; chunkY <= 12; ++chunkY)
    {
        for (int chunkX = -12; chunkX <= 12; ++chunkX)
        {
            const rpg::ChunkContent content = world.getChunkContent(rpg::ChunkCoordinates{chunkX, chunkY});

            for (const rpg::ContentInstance& instance : content.instances)
            {
                const rpg::detail::VegetationPrototype& prototype = metadata.getPrototypeById(instance.prototypeId);
                const float expectedOffsetX = (static_cast<float>(prototype.bounds.minOffsetX) - 0.5F) * world.getTileSize();
                const float expectedOffsetY = (static_cast<float>(prototype.bounds.minOffsetY) - 0.5F) * world.getTileSize();
                const float expectedWidth = static_cast<float>(
                    prototype.bounds.maxOffsetX - prototype.bounds.minOffsetX + 1) * world.getTileSize();
                const float expectedHeight = static_cast<float>(
                    prototype.bounds.maxOffsetY - prototype.bounds.minOffsetY + 1) * world.getTileSize();

                if (!areClose(instance.footprint.offset.x, expectedOffsetX)
                    || !areClose(instance.footprint.offset.y, expectedOffsetY)
                    || !areClose(instance.footprint.size.width, expectedWidth)
                    || !areClose(instance.footprint.size.height, expectedHeight))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

[[nodiscard]] bool verifyAbsoluteCoordinateSignalsAreWorldSizeIndependent()
{
    const rpg::WorldConfig smallerWorldConfig{
        .seed = 0x55AA12EFU,
        .widthInTiles = 40,
        .heightInTiles = 24,
        .tileSize = 24.0F};
    const rpg::WorldConfig largerWorldConfig{
        .seed = smallerWorldConfig.seed,
        .widthInTiles = 80,
        .heightInTiles = 64,
        .tileSize = smallerWorldConfig.tileSize};
    const std::array<rpg::TileCoordinates, 4> sampleCoordinates = {{
        {80, 64},
        {96, 64},
        {-32, 48},
        {120, -24},
    }};

    const rpg::World smallerWorld(smallerWorldConfig);
    const rpg::World largerWorld(largerWorldConfig);

    for (const rpg::TileCoordinates& coordinates : sampleCoordinates)
    {
        if (smallerWorld.getTileType(coordinates) != largerWorld.getTileType(coordinates))
        {
            return false;
        }
    }

    return true;
}

[[nodiscard]] bool verifyUnsupportedTerrainShapesAreRemoved()
{
    const std::array<rpg::WorldConfig, 3> configs = {{
        {.seed = 0x12345678U, .widthInTiles = 40, .heightInTiles = 24, .tileSize = 24.0F},
        {.seed = 0x13572468U, .widthInTiles = 36, .heightInTiles = 20, .tileSize = 20.0F},
        {.seed = 0x89ABCDEFU, .widthInTiles = 48, .heightInTiles = 28, .tileSize = 16.0F},
    }};
    const std::array<rpg::TileCoordinates, 3> windowAnchors = {{
        {0, 0},
        {rpg::detail::getChunkSizeInTiles() * 7, -rpg::detail::getChunkSizeInTiles() * 5},
        {-rpg::detail::getChunkSizeInTiles() * 6, rpg::detail::getChunkSizeInTiles() * 4},
    }};
    constexpr int kHalfWindowSizeInTiles = rpg::detail::getChunkSizeInTiles();

    for (const rpg::WorldConfig& config : configs)
    {
        const rpg::World world(config);

        for (const rpg::TileCoordinates& anchor : windowAnchors)
        {
            for (int y = anchor.y - kHalfWindowSizeInTiles; y <= anchor.y + kHalfWindowSizeInTiles; ++y)
            {
                for (int x = anchor.x - kHalfWindowSizeInTiles; x <= anchor.x + kHalfWindowSizeInTiles; ++x)
                {
                    if (hasUnsupportedTerrainShape(world, {x, y}))
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

} // namespace

int main()
{
    if (!verifyDeterministicGeneration())
    {
        return 1;
    }

    if (!verifyDeterministicBiomeSampling())
    {
        return 1;
    }

    if (!verifyDeterministicChunkGeneration())
    {
        return 1;
    }

    if (!verifyGroundContentAvoidsRoadCoveredTiles())
    {
        return 1;
    }

    if (!verifyVisibleRoadOverlaysPreserveTopologyBackedEndpoints())
    {
        return 1;
    }

    if (!verifyRoadNetworkGenerationIsDeterministic())
    {
        return 1;
    }

    if (!verifyRoadNetworkConnectsSpawnAndSupportsDeterministicLayouts())
    {
        return 1;
    }

    if (!verifyRoadNetworkPublishesDeterministicStampMetadata())
    {
        return 1;
    }

    if (!verifyStampedRoadSupportExpandsMainRoadsAndJunctions())
    {
        return 1;
    }

    if (!verifyStampedRoadSupportKeepsBendsConnected())
    {
        return 1;
    }

    if (!verifySharedRoadOverlaySupportAvoidsSingleTileNotches())
    {
        return 1;
    }

    if (!verifyOriginAnchoredSpawnSelection())
    {
        return 1;
    }

    if (!verifySpawnValidity())
    {
        return 1;
    }

    if (!verifyCoastlinesAndInlandBiomes())
    {
        return 1;
    }

    if (!verifyNegativeChunkCoordinateMapping())
    {
        return 1;
    }

    if (!verifyChunkGenerationExtendsBeyondInitialArea())
    {
        return 1;
    }

    if (!verifyVisibleRenderQueriesGenerateAndReuseWorldCache())
    {
        return 1;
    }

    if (!verifyMetadataQueriesGenerateAndReuseWorldCache())
    {
        return 1;
    }

    if (!verifyBorrowedMetadataQueriesExposeRetainedChunkData())
    {
        return 1;
    }

    if (!verifyContentQueriesGenerateAndReuseWorldCache())
    {
        return 1;
    }

    if (!verifyBorrowedContentQueriesExposeRetainedChunkData())
    {
        return 1;
    }

    if (!verifyMissingChunkLoadsReuseRetainedGenerationHelpers())
    {
        return 1;
    }

    if (!verifyChunkContentRecords())
    {
        return 1;
    }

    if (!verifyVisibleContentQueries())
    {
        return 1;
    }

    if (!verifyChunkRetentionWindowCanUnloadAndRegenerate())
    {
        return 1;
    }

    if (!verifyVisibleQueriesStillWorkWithChunkUnloading())
    {
        return 1;
    }

    if (!verifyForestChunksProduceDenserVegetationThanGrassChunks())
    {
        return 1;
    }

    if (!verifyWaterVegetationPlacementIsDeterministic())
    {
        return 1;
    }

    if (!verifyVegetationPlacementRespectsPrototypeConstraints())
    {
        return 1;
    }

    if (!verifyForestPlacementModeDensityHierarchy())
    {
        return 1;
    }

    if (!verifyLargeVegetationRemainsVisibleAcrossChunkBoundaries())
    {
        return 1;
    }

    if (!verifyVegetationFootprintsMatchMetadataBounds())
    {
        return 1;
    }

    if (!verifyAbsoluteCoordinateSignalsAreWorldSizeIndependent())
    {
        return 1;
    }

    if (!verifyUnsupportedTerrainShapesAreRemoved())
    {
        return 1;
    }

    return 0;
}
