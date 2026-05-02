/**
 * @file TerrainAutotileSupport.cpp
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

#include "TerrainAutotileSupport.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <limits>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>

namespace rpg
{
namespace detail
{

namespace
{

constexpr int kWaterAnimationFrameDurationMilliseconds = 200;
constexpr std::uint32_t kBaseVariantHashSalt = 0x68E31DA4U;
constexpr std::uint32_t kDecorRollHashSalt = 0x1F2A5C7DU;
constexpr std::uint32_t kDecorVariantHashSalt = 0x9B3C41E2U;
constexpr std::uint32_t kDecorWeightNumerator = 1U;
constexpr std::uint32_t kDecorWeightDenominator = 32U;

[[nodiscard]] std::string readFileContents(const std::filesystem::path& path)
{
    std::ifstream input(path);

    if (!input)
    {
        throw std::runtime_error("Failed to open terrain tileset classification file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

[[nodiscard]] std::vector<std::string> extractJsonObjects(const std::string& contents)
{
    std::vector<std::string> objects;
    std::size_t objectStart = std::string::npos;
    int depth = 0;
    bool inString = false;
    bool escaped = false;

    for (std::size_t index = 0; index < contents.size(); ++index)
    {
        const char character = contents[index];

        if (escaped)
        {
            escaped = false;
            continue;
        }

        if (character == '\\' && inString)
        {
            escaped = true;
            continue;
        }

        if (character == '"')
        {
            inString = !inString;
            continue;
        }

        if (inString)
        {
            continue;
        }

        if (character == '{')
        {
            if (depth == 0)
            {
                objectStart = index;
            }

            ++depth;
            continue;
        }

        if (character == '}')
        {
            --depth;

            if (depth == 0 && objectStart != std::string::npos)
            {
                objects.push_back(contents.substr(objectStart, index - objectStart + 1));
                objectStart = std::string::npos;
            }
        }
    }

    return objects;
}

[[nodiscard]] std::optional<std::string> matchStringField(
    const std::string& objectText,
    const std::string& fieldName)
{
    const std::regex pattern("\"" + fieldName + "\"\\s*:\\s*(null|\"([^\"]*)\")");
    std::smatch match;

    if (!std::regex_search(objectText, match, pattern))
    {
        return std::nullopt;
    }

    if (match[1].str() == "null")
    {
        return std::nullopt;
    }

    return match[2].str();
}

[[nodiscard]] int matchRequiredIntField(const std::string& objectText, const std::string& fieldName)
{
    const std::regex pattern("\"" + fieldName + "\"\\s*:\\s*(-?\\d+)");
    std::smatch match;

    if (!std::regex_search(objectText, match, pattern))
    {
        throw std::runtime_error("Missing integer field in terrain tileset classification: " + fieldName);
    }

    return std::stoi(match[1].str());
}

[[nodiscard]] std::optional<int> matchOptionalIntField(const std::string& objectText, const std::string& fieldName)
{
    const std::regex pattern("\"" + fieldName + "\"\\s*:\\s*(null|-?\\d+)");
    std::smatch match;

    if (!std::regex_search(objectText, match, pattern))
    {
        return std::nullopt;
    }

    if (match[1].str() == "null")
    {
        return std::nullopt;
    }

    return std::stoi(match[1].str());
}

[[nodiscard]] TileType parseTileType(const std::string& value)
{
    if (value == "grass")
    {
        return TileType::Grass;
    }

    if (value == "sand")
    {
        return TileType::Sand;
    }

    if (value == "water")
    {
        return TileType::Water;
    }

    if (value == "forest")
    {
        return TileType::Forest;
    }

    throw std::runtime_error("Unknown terrain type in terrain tileset classification: " + value);
}

[[nodiscard]] TerrainAutotileRole parseAutotileRole(const std::string& value)
{
    if (value == "outer_top_left")
    {
        return TerrainAutotileRole::OuterTopLeft;
    }

    if (value == "outer_top_right")
    {
        return TerrainAutotileRole::OuterTopRight;
    }

    if (value == "outer_bottom_left")
    {
        return TerrainAutotileRole::OuterBottomLeft;
    }

    if (value == "outer_bottom_right")
    {
        return TerrainAutotileRole::OuterBottomRight;
    }

    if (value == "single_tile")
    {
        return TerrainAutotileRole::SingleTile;
    }

    if (value == "top_left")
    {
        return TerrainAutotileRole::TopLeft;
    }

    if (value == "top")
    {
        return TerrainAutotileRole::Top;
    }

    if (value == "top_right")
    {
        return TerrainAutotileRole::TopRight;
    }

    if (value == "left")
    {
        return TerrainAutotileRole::Left;
    }

    if (value == "center")
    {
        return TerrainAutotileRole::Center;
    }

    if (value == "right")
    {
        return TerrainAutotileRole::Right;
    }

    if (value == "bottom_left")
    {
        return TerrainAutotileRole::BottomLeft;
    }

    if (value == "bottom")
    {
        return TerrainAutotileRole::Bottom;
    }

    if (value == "bottom_right")
    {
        return TerrainAutotileRole::BottomRight;
    }

    throw std::runtime_error("Unknown autotile role in terrain tileset classification: " + value);
}

[[nodiscard]] constexpr std::array<TerrainAutotileRole, 14> getAllAutotileRoles() noexcept
{
    return {
        TerrainAutotileRole::OuterTopLeft,
        TerrainAutotileRole::OuterTopRight,
        TerrainAutotileRole::OuterBottomLeft,
        TerrainAutotileRole::OuterBottomRight,
        TerrainAutotileRole::SingleTile,
        TerrainAutotileRole::TopLeft,
        TerrainAutotileRole::Top,
        TerrainAutotileRole::TopRight,
        TerrainAutotileRole::Left,
        TerrainAutotileRole::Center,
        TerrainAutotileRole::Right,
        TerrainAutotileRole::BottomLeft,
        TerrainAutotileRole::Bottom,
        TerrainAutotileRole::BottomRight,
    };
}

[[nodiscard]] constexpr std::array<std::pair<TileType, TileType>, 6> getSupportedTransitionPairs() noexcept
{
    return {{
        {TileType::Grass, TileType::Forest},
        {TileType::Grass, TileType::Sand},
        {TileType::Grass, TileType::Water},
        {TileType::Forest, TileType::Sand},
        {TileType::Forest, TileType::Water},
        {TileType::Sand, TileType::Water},
    }};
}

void validateBaseVariants(const TerrainTilesetMetadata& metadata)
{
    for (const TileType tileType : {TileType::Grass, TileType::Sand, TileType::Forest})
    {
        if (metadata.getBaseVariantCount(tileType) == 0)
        {
            throw std::runtime_error("Missing required base variants in terrain tileset classification");
        }
    }
}

void validateTransitionEntries(const TerrainTilesetMetadata& metadata)
{
    if (metadata.getWaterAnimationFrameCount() <= 0)
    {
        throw std::runtime_error("Missing water animation frames in terrain tileset classification");
    }

    for (const auto& [sourceTileType, transitionTarget] : getSupportedTransitionPairs())
    {
        for (const TerrainAutotileRole role : getAllAutotileRoles())
        {
            if (transitionTarget == TileType::Water)
            {
                for (int animationFrame = 0; animationFrame < metadata.getWaterAnimationFrameCount(); ++animationFrame)
                {
                    (void)metadata.getTransitionCell(sourceTileType, transitionTarget, role, animationFrame);
                }
            }
            else
            {
                (void)metadata.getTransitionCell(sourceTileType, transitionTarget, role);
            }
        }
    }

    for (int animationFrame = 0; animationFrame < metadata.getWaterAnimationFrameCount(); ++animationFrame)
    {
        (void)metadata.getOpenWaterCell(animationFrame);
    }
}

[[nodiscard]] bool areAllNeighborsTarget(
    const bool north,
    const bool northEast,
    const bool east,
    const bool southEast,
    const bool south,
    const bool southWest,
    const bool west,
    const bool northWest) noexcept
{
    return north && northEast && east && southEast && south && southWest && west && northWest;
}

struct TransitionSourceScore
{
    int cardinalMatches = 0;
    int totalMatches = 0;
    int priority = std::numeric_limits<int>::min();
};

[[nodiscard]] TransitionSourceScore scoreTransitionSource(
    const TileType sourceTileType,
    const std::array<TileType, 8>& neighborTileTypes) noexcept
{
    TransitionSourceScore score;
    score.priority = getTerrainPriority(sourceTileType);

    for (std::size_t index = 0; index < neighborTileTypes.size(); ++index)
    {
        if (neighborTileTypes[index] != sourceTileType)
        {
            continue;
        }

        ++score.totalMatches;

        if (index == 0 || index == 2 || index == 4 || index == 6)
        {
            ++score.cardinalMatches;
        }
    }

    return score;
}

[[nodiscard]] bool isBetterTransitionSource(
    const TransitionSourceScore& candidate,
    const TransitionSourceScore& bestSoFar) noexcept
{
    return std::tie(candidate.cardinalMatches, candidate.totalMatches, candidate.priority)
        > std::tie(bestSoFar.cardinalMatches, bestSoFar.totalMatches, bestSoFar.priority);
}

[[nodiscard]] constexpr bool isTransitionOccupant(
    const TileType currentTileType,
    const TileType neighborTileType) noexcept
{
    return supportsAutotileTransition(neighborTileType, currentTileType);
}

[[nodiscard]] constexpr TerrainAutotileRole reverseAutotileOrientation(const TerrainAutotileRole role) noexcept
{
    return role;
}

[[nodiscard]] std::uint32_t hashTerrainSelection(
    const std::uint32_t seed,
    const TileCoordinates& coordinates,
    const TileType tileType,
    const std::uint32_t salt) noexcept
{
    std::uint32_t value = seed ^ salt;
    value ^= static_cast<std::uint32_t>(coordinates.x) * 73856093U;
    value ^= static_cast<std::uint32_t>(coordinates.y) * 19349663U;
    value ^= static_cast<std::uint32_t>(getTerrainPriority(tileType)) * 83492791U;
    value ^= value >> 13U;
    value *= 1274126177U;
    value ^= value >> 16U;
    return value;
}

[[nodiscard]] TerrainAtlasCell selectDefaultTerrainAtlasCell(
    const TerrainTilesetMetadata& metadata,
    const std::uint32_t seed,
    const TileCoordinates& coordinates,
    const TileType tileType)
{
    const TerrainAppearanceSelection selection = selectTerrainAppearanceSelection(
        seed,
        coordinates,
        tileType,
        metadata.getBaseVariantCount(tileType),
        metadata.getDecorVariantCount(tileType));

    return selection.useDecor
        ? metadata.getDecorVariant(tileType, selection.variantIndex)
        : metadata.getBaseVariant(tileType, selection.variantIndex);
}

} // namespace

bool TerrainTilesetMetadata::TransitionKey::operator<(const TransitionKey& other) const noexcept
{
    return std::tie(sourceTileType, transitionTarget) < std::tie(other.sourceTileType, other.transitionTarget);
}

TerrainTilesetMetadata TerrainTilesetMetadata::loadFromFile(const std::filesystem::path& path)
{
    TerrainTilesetMetadata metadata;
    const std::vector<std::string> objects = extractJsonObjects(readFileContents(path));

    for (const std::string& objectText : objects)
    {
        const std::optional<std::string> category = matchStringField(objectText, "category");

        if (!category.has_value())
        {
            continue;
        }

        const TerrainAtlasCell cell{
            matchRequiredIntField(objectText, "x"),
            matchRequiredIntField(objectText, "y")};

        if (*category == "base")
        {
            const std::optional<std::string> terrainValue = matchStringField(objectText, "terrain");

            if (!terrainValue.has_value())
            {
                throw std::runtime_error("Base terrain entry is missing terrain value");
            }

            metadata.m_baseVariants[parseTileType(*terrainValue)].push_back(cell);
            continue;
        }

        if (*category == "decor")
        {
            const std::optional<std::string> terrainValue = matchStringField(objectText, "terrain");

            if (!terrainValue.has_value())
            {
                throw std::runtime_error("Decor terrain entry is missing terrain value");
            }

            metadata.m_decorVariants[parseTileType(*terrainValue)].push_back(cell);
            continue;
        }

        if (*category != "autotile_transition")
        {
            continue;
        }

        const std::optional<std::string> terrainValue = matchStringField(objectText, "terrain");
        const std::optional<std::string> transitionToValue = matchStringField(objectText, "transitionTo");
        const std::optional<std::string> autotileRoleValue = matchStringField(objectText, "autotileRole");

        if (!terrainValue.has_value() || !transitionToValue.has_value() || !autotileRoleValue.has_value())
        {
            throw std::runtime_error("Autotile transition entry is missing required terrain metadata");
        }

        const TransitionKey key{
            parseTileType(*terrainValue),
            parseTileType(*transitionToValue)};
        const TerrainAutotileRole role = parseAutotileRole(*autotileRoleValue);
        const int animationFrame = matchOptionalIntField(objectText, "animationFrame").value_or(0);
        metadata.m_transitionCells[key][role][animationFrame] = cell;
    }

    validateBaseVariants(metadata);
    validateTransitionEntries(metadata);
    return metadata;
}

std::size_t TerrainTilesetMetadata::getBaseVariantCount(const TileType tileType) const noexcept
{
    const auto baseVariantsIt = m_baseVariants.find(tileType);
    return baseVariantsIt == m_baseVariants.end() ? 0U : baseVariantsIt->second.size();
}

const TerrainAtlasCell& TerrainTilesetMetadata::getBaseVariant(
    const TileType tileType,
    const std::size_t variantIndex) const
{
    const auto baseVariantsIt = m_baseVariants.find(tileType);

    if (baseVariantsIt == m_baseVariants.end() || baseVariantsIt->second.empty())
    {
        throw std::runtime_error("Missing terrain base variants for requested tile type");
    }

    return baseVariantsIt->second[variantIndex % baseVariantsIt->second.size()];
}

std::size_t TerrainTilesetMetadata::getDecorVariantCount(const TileType tileType) const noexcept
{
    const auto decorVariantsIt = m_decorVariants.find(tileType);
    return decorVariantsIt == m_decorVariants.end() ? 0U : decorVariantsIt->second.size();
}

const TerrainAtlasCell& TerrainTilesetMetadata::getDecorVariant(
    const TileType tileType,
    const std::size_t variantIndex) const
{
    const auto decorVariantsIt = m_decorVariants.find(tileType);

    if (decorVariantsIt == m_decorVariants.end() || decorVariantsIt->second.empty())
    {
        throw std::runtime_error("Missing terrain decor variants for requested tile type");
    }

    return decorVariantsIt->second[variantIndex % decorVariantsIt->second.size()];
}

const TerrainAtlasCell& TerrainTilesetMetadata::getTransitionCell(
    const TileType sourceTileType,
    const TileType transitionTarget,
    const TerrainAutotileRole role,
    const int animationFrame) const
{
    const TransitionKey key{sourceTileType, transitionTarget};
    const auto transitionIt = m_transitionCells.find(key);

    if (transitionIt == m_transitionCells.end())
    {
        throw std::runtime_error("Missing terrain transition pair in terrain tileset classification");
    }

    const auto roleIt = transitionIt->second.find(role);

    if (roleIt == transitionIt->second.end())
    {
        throw std::runtime_error("Missing autotile role in terrain tileset classification");
    }

    const auto frameIt = roleIt->second.find(animationFrame);

    if (frameIt == roleIt->second.end())
    {
        throw std::runtime_error("Missing animation frame in terrain tileset classification");
    }

    return frameIt->second;
}

const TerrainAtlasCell& TerrainTilesetMetadata::getOpenWaterCell(const int animationFrame) const
{
    return getTransitionCell(TileType::Grass, TileType::Water, TerrainAutotileRole::Center, animationFrame);
}

int TerrainTilesetMetadata::getWaterAnimationFrameCount() const noexcept
{
    const TransitionKey key{TileType::Grass, TileType::Water};
    const auto transitionIt = m_transitionCells.find(key);

    if (transitionIt == m_transitionCells.end())
    {
        return 0;
    }

    const auto roleIt = transitionIt->second.find(TerrainAutotileRole::Center);
    return roleIt == transitionIt->second.end() ? 0 : static_cast<int>(roleIt->second.size());
}

std::optional<TileType> selectAutotileTransitionTarget(
    const TileType currentTileType,
    const std::array<TileType, 8>& neighborTileTypes) noexcept
{
    std::optional<TileType> selectedTarget;
    TransitionSourceScore bestScore;

    for (const TileType neighborTileType : neighborTileTypes)
    {
        if (!supportsAutotileTransition(neighborTileType, currentTileType))
        {
            continue;
        }

        const TransitionSourceScore candidateScore = scoreTransitionSource(neighborTileType, neighborTileTypes);

        if (!selectedTarget.has_value() || isBetterTransitionSource(candidateScore, bestScore))
        {
            bestScore = candidateScore;
            selectedTarget = neighborTileType;
        }
    }

    return selectedTarget;
}

std::optional<TerrainAutotileRole> resolveAutotileRole(
    const TileType currentTileType,
    const TileType transitionSource,
    const std::array<TileType, 8>& neighborTileTypes) noexcept
{
    const bool north = isTransitionOccupant(currentTileType, neighborTileTypes[0]);
    const bool northEast = isTransitionOccupant(currentTileType, neighborTileTypes[1]);
    const bool east = isTransitionOccupant(currentTileType, neighborTileTypes[2]);
    const bool southEast = isTransitionOccupant(currentTileType, neighborTileTypes[3]);
    const bool south = isTransitionOccupant(currentTileType, neighborTileTypes[4]);
    const bool southWest = isTransitionOccupant(currentTileType, neighborTileTypes[5]);
    const bool west = isTransitionOccupant(currentTileType, neighborTileTypes[6]);
    const bool northWest = isTransitionOccupant(currentTileType, neighborTileTypes[7]);
    const bool exactNorth = neighborTileTypes[0] == transitionSource;
    const bool exactNorthEast = neighborTileTypes[1] == transitionSource;
    const bool exactEast = neighborTileTypes[2] == transitionSource;
    const bool exactSouthEast = neighborTileTypes[3] == transitionSource;
    const bool exactSouth = neighborTileTypes[4] == transitionSource;
    const bool exactSouthWest = neighborTileTypes[5] == transitionSource;
    const bool exactWest = neighborTileTypes[6] == transitionSource;
    const bool exactNorthWest = neighborTileTypes[7] == transitionSource;

    if (areAllNeighborsTarget(
            exactNorth,
            exactNorthEast,
            exactEast,
            exactSouthEast,
            exactSouth,
            exactSouthWest,
            exactWest,
            exactNorthWest))
    {
        return TerrainAutotileRole::SingleTile;
    }

    if (north && east && south && west)
    {
        return TerrainAutotileRole::Center;
    }

    if (north && east && west)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::Top);
    }

    if (south && east && west)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::Bottom);
    }

    if (north && south && west)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::Left);
    }

    if (north && south && east)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::Right);
    }

    if (north && west && !east && !south)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::TopLeft);
    }

    if (north && east && !west && !south)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::TopRight);
    }

    if (south && west && !north && !east)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::BottomLeft);
    }

    if (south && east && !north && !west)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::BottomRight);
    }

    if (north && !east && !south && !west)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::Top);
    }

    if (south && !north && !east && !west)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::Bottom);
    }

    if (west && !north && !east && !south)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::Left);
    }

    if (east && !north && !south && !west)
    {
        return reverseAutotileOrientation(TerrainAutotileRole::Right);
    }

    if (!north && !east && !south && !west)
    {
        if (northWest && !northEast && !southWest && !southEast)
        {
            return reverseAutotileOrientation(TerrainAutotileRole::OuterTopLeft);
        }

        if (northEast && !northWest && !southWest && !southEast)
        {
            return reverseAutotileOrientation(TerrainAutotileRole::OuterTopRight);
        }

        if (southWest && !northWest && !northEast && !southEast)
        {
            return reverseAutotileOrientation(TerrainAutotileRole::OuterBottomLeft);
        }

        if (southEast && !northWest && !northEast && !southWest)
        {
            return reverseAutotileOrientation(TerrainAutotileRole::OuterBottomRight);
        }

        return std::nullopt;
    }

    return TerrainAutotileRole::Center;
}

std::size_t selectTerrainVariantIndex(
    const std::uint32_t seed,
    const TileCoordinates& coordinates,
    const TileType tileType,
    const std::size_t variantCount,
    const std::uint32_t salt) noexcept
{
    if (variantCount == 0)
    {
        return 0;
    }

    return static_cast<std::size_t>(hashTerrainSelection(seed, coordinates, tileType, salt) % variantCount);
}

TerrainAppearanceSelection selectTerrainAppearanceSelection(
    const std::uint32_t seed,
    const TileCoordinates& coordinates,
    const TileType tileType,
    const std::size_t baseVariantCount,
    const std::size_t decorVariantCount) noexcept
{
    TerrainAppearanceSelection selection;

    if (baseVariantCount == 0)
    {
        return selection;
    }

    const bool useDecor = decorVariantCount > 0
        && (hashTerrainSelection(seed, coordinates, tileType, kDecorRollHashSalt) % kDecorWeightDenominator) < kDecorWeightNumerator;
    selection.useDecor = useDecor;
    selection.variantIndex = selectTerrainVariantIndex(
        seed,
        coordinates,
        tileType,
        useDecor ? decorVariantCount : baseVariantCount,
        useDecor ? kDecorVariantHashSalt : kBaseVariantHashSalt);
    return selection;
}

int selectWaterAnimationFrame(const float elapsedSeconds, const int frameCount) noexcept
{
    if (frameCount <= 0)
    {
        return 0;
    }

    const auto elapsedMilliseconds = static_cast<std::int64_t>(std::max(elapsedSeconds, 0.0F) * 1000.0F);
    const auto frameIndex = (elapsedMilliseconds / kWaterAnimationFrameDurationMilliseconds) % frameCount;
    return static_cast<int>(frameIndex);
}

TerrainAtlasCell selectTerrainAtlasCell(
    const TerrainTilesetMetadata& metadata,
    const OverworldRenderTile& tile,
    const std::array<TileType, 8>& neighborTileTypes,
    const float animationElapsedSeconds,
    const std::uint32_t seed)
{
    const std::optional<TileType> transitionSource = selectAutotileTransitionTarget(tile.tileType, neighborTileTypes);

    if (tile.tileType == TileType::Water)
    {
        if (!transitionSource.has_value())
        {
            return metadata.getOpenWaterCell(selectWaterAnimationFrame(
                animationElapsedSeconds,
                metadata.getWaterAnimationFrameCount()));
        }

        const std::optional<TerrainAutotileRole> role =
            resolveAutotileRole(tile.tileType, *transitionSource, neighborTileTypes);

        if (!role.has_value())
        {
            return metadata.getOpenWaterCell(selectWaterAnimationFrame(
                animationElapsedSeconds,
                metadata.getWaterAnimationFrameCount()));
        }

        return metadata.getTransitionCell(
            *transitionSource,
            tile.tileType,
            *role,
            selectWaterAnimationFrame(animationElapsedSeconds, metadata.getWaterAnimationFrameCount()));
    }

    if (!transitionSource.has_value())
    {
        return selectDefaultTerrainAtlasCell(metadata, seed, tile.coordinates, tile.tileType);
    }

    const std::optional<TerrainAutotileRole> role =
        resolveAutotileRole(tile.tileType, *transitionSource, neighborTileTypes);

    if (!role.has_value())
    {
        return selectDefaultTerrainAtlasCell(metadata, seed, tile.coordinates, tile.tileType);
    }

    return metadata.getTransitionCell(
        *transitionSource,
        tile.tileType,
        *role);
}

} // namespace detail
} // namespace rpg
