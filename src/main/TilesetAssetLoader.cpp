/**
 * @file TilesetAssetLoader.cpp
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

#include "TilesetAssetLoader.hpp"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace rpg
{
namespace detail
{

namespace
{

struct JsonValue;

using JsonObject = std::map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;

struct JsonValue
{
    std::variant<std::nullptr_t, bool, double, std::string, JsonObject, JsonArray> value;
};

[[nodiscard]] std::string readFileContents(const std::filesystem::path& path)
{
    std::ifstream input(path);

    if (!input)
    {
        throw std::runtime_error("Failed to open tileset catalog file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

class JsonParser
{
public:
    explicit JsonParser(const std::string& text)
        : m_text(text)
    {
    }

    [[nodiscard]] JsonValue parse()
    {
        skipWhitespace();
        JsonValue value = parseValue();
        skipWhitespace();

        if (m_index != m_text.size())
        {
            throw std::runtime_error("Unexpected trailing content in tileset catalog");
        }

        return value;
    }

private:
    [[nodiscard]] JsonValue parseValue()
    {
        if (m_index >= m_text.size())
        {
            throw std::runtime_error("Unexpected end of tileset catalog");
        }

        const char character = m_text[m_index];

        if (character == '{')
        {
            return JsonValue{parseObject()};
        }

        if (character == '[')
        {
            return JsonValue{parseArray()};
        }

        if (character == '"')
        {
            return JsonValue{parseString()};
        }

        if (character == 't')
        {
            consumeLiteral("true");
            return JsonValue{true};
        }

        if (character == 'f')
        {
            consumeLiteral("false");
            return JsonValue{false};
        }

        if (character == 'n')
        {
            consumeLiteral("null");
            return JsonValue{nullptr};
        }

        if (character == '-' || std::isdigit(static_cast<unsigned char>(character)))
        {
            return JsonValue{parseNumber()};
        }

        throw std::runtime_error("Unexpected token in tileset catalog");
    }

    [[nodiscard]] JsonObject parseObject()
    {
        expect('{');
        skipWhitespace();

        JsonObject object;

        if (consumeIf('}'))
        {
            return object;
        }

        while (true)
        {
            skipWhitespace();
            const std::string key = parseString();
            skipWhitespace();
            expect(':');
            skipWhitespace();
            object.emplace(key, parseValue());
            skipWhitespace();

            if (consumeIf('}'))
            {
                return object;
            }

            expect(',');
            skipWhitespace();
        }
    }

    [[nodiscard]] JsonArray parseArray()
    {
        expect('[');
        skipWhitespace();

        JsonArray array;

        if (consumeIf(']'))
        {
            return array;
        }

        while (true)
        {
            skipWhitespace();
            array.push_back(parseValue());
            skipWhitespace();

            if (consumeIf(']'))
            {
                return array;
            }

            expect(',');
            skipWhitespace();
        }
    }

    [[nodiscard]] std::string parseString()
    {
        expect('"');
        std::string value;

        while (m_index < m_text.size())
        {
            const char character = m_text[m_index++];

            if (character == '"')
            {
                return value;
            }

            if (character != '\\')
            {
                value.push_back(character);
                continue;
            }

            if (m_index >= m_text.size())
            {
                throw std::runtime_error("Invalid escape sequence in tileset catalog");
            }

            const char escaped = m_text[m_index++];

            switch (escaped)
            {
            case '"':
            case '\\':
            case '/':
                value.push_back(escaped);
                break;
            case 'b':
                value.push_back('\b');
                break;
            case 'f':
                value.push_back('\f');
                break;
            case 'n':
                value.push_back('\n');
                break;
            case 'r':
                value.push_back('\r');
                break;
            case 't':
                value.push_back('\t');
                break;
            default:
                throw std::runtime_error("Unsupported escape sequence in tileset catalog");
            }
        }

        throw std::runtime_error("Unterminated string in tileset catalog");
    }

    [[nodiscard]] double parseNumber()
    {
        const std::size_t start = m_index;

        if (m_text[m_index] == '-')
        {
            ++m_index;
        }

        if (m_index >= m_text.size() || !std::isdigit(static_cast<unsigned char>(m_text[m_index])))
        {
            throw std::runtime_error("Invalid number in tileset catalog");
        }

        while (m_index < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_index])))
        {
            ++m_index;
        }

        if (m_index < m_text.size() && m_text[m_index] == '.')
        {
            ++m_index;

            if (m_index >= m_text.size() || !std::isdigit(static_cast<unsigned char>(m_text[m_index])))
            {
                throw std::runtime_error("Invalid number in tileset catalog");
            }

            while (m_index < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_index])))
            {
                ++m_index;
            }
        }

        return std::stod(m_text.substr(start, m_index - start));
    }

    void consumeLiteral(const char* literal)
    {
        const std::string literalString(literal);

        if (m_text.compare(m_index, literalString.size(), literalString) != 0)
        {
            throw std::runtime_error("Invalid literal in tileset catalog");
        }

        m_index += literalString.size();
    }

    void skipWhitespace()
    {
        while (m_index < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_index])))
        {
            ++m_index;
        }
    }

    void expect(const char expected)
    {
        if (m_index >= m_text.size() || m_text[m_index] != expected)
        {
            throw std::runtime_error("Unexpected token in tileset catalog");
        }

        ++m_index;
    }

    [[nodiscard]] bool consumeIf(const char expected)
    {
        if (m_index < m_text.size() && m_text[m_index] == expected)
        {
            ++m_index;
            return true;
        }

        return false;
    }

    const std::string& m_text;
    std::size_t m_index = 0;
};

[[nodiscard]] const JsonObject& asObject(const JsonValue& value, const std::string_view context)
{
    const auto* object = std::get_if<JsonObject>(&value.value);

    if (!object)
    {
        throw std::runtime_error("Expected object in tileset catalog: " + std::string(context));
    }

    return *object;
}

[[nodiscard]] std::filesystem::path inferAssetRootFromCatalogPath(const std::filesystem::path& catalogPath)
{
    const std::filesystem::path catalogDirectory = catalogPath.parent_path().lexically_normal();

    if (catalogDirectory.filename() == "catalogs" && catalogDirectory.parent_path().filename() == "output")
    {
        return catalogDirectory.parent_path().parent_path();
    }

    return catalogDirectory;
}

[[nodiscard]] const JsonArray& asArray(const JsonValue& value, const std::string_view context)
{
    const auto* array = std::get_if<JsonArray>(&value.value);

    if (!array)
    {
        throw std::runtime_error("Expected array in tileset catalog: " + std::string(context));
    }

    return *array;
}

[[nodiscard]] const std::string& asString(const JsonValue& value, const std::string_view context)
{
    const auto* stringValue = std::get_if<std::string>(&value.value);

    if (!stringValue)
    {
        throw std::runtime_error("Expected string in tileset catalog: " + std::string(context));
    }

    return *stringValue;
}

[[nodiscard]] bool asBool(const JsonValue& value, const std::string_view context)
{
    const auto* boolValue = std::get_if<bool>(&value.value);

    if (!boolValue)
    {
        throw std::runtime_error("Expected boolean in tileset catalog: " + std::string(context));
    }

    return *boolValue;
}

[[nodiscard]] int asInt(const JsonValue& value, const std::string_view context)
{
    const auto* numberValue = std::get_if<double>(&value.value);

    if (!numberValue)
    {
        throw std::runtime_error("Expected integer in tileset catalog: " + std::string(context));
    }

    const int integerValue = static_cast<int>(*numberValue);

    if (static_cast<double>(integerValue) != *numberValue)
    {
        throw std::runtime_error("Expected integer in tileset catalog: " + std::string(context));
    }

    return integerValue;
}

[[nodiscard]] float asFloat(const JsonValue& value, const std::string_view context)
{
    const auto* numberValue = std::get_if<double>(&value.value);

    if (!numberValue)
    {
        throw std::runtime_error("Expected number in tileset catalog: " + std::string(context));
    }

    return static_cast<float>(*numberValue);
}

    [[nodiscard]] const JsonValue& getRequiredField(
    const JsonObject& object,
    const std::string_view fieldName,
    const std::string_view context)
{
    const auto fieldIt = object.find(std::string(fieldName));

    if (fieldIt == object.end())
    {
        throw std::runtime_error(
            "Missing field in tileset catalog: " + std::string(context) + "." + std::string(fieldName));
    }

    return fieldIt->second;
}

[[nodiscard]] const JsonValue* getOptionalField(const JsonObject& object, const std::string_view fieldName)
{
    const auto fieldIt = object.find(std::string(fieldName));
    return fieldIt == object.end() ? nullptr : &fieldIt->second;
}

[[nodiscard]] std::optional<std::string> getOptionalString(const JsonObject& object, const std::string_view fieldName)
{
    const JsonValue* field = getOptionalField(object, fieldName);

    if (!field)
    {
        return std::nullopt;
    }

    if (std::holds_alternative<std::nullptr_t>(field->value))
    {
        return std::nullopt;
    }

    return asString(*field, fieldName);
}

[[nodiscard]] std::optional<int> getOptionalInt(const JsonObject& object, const std::string_view fieldName)
{
    const JsonValue* field = getOptionalField(object, fieldName);

    if (!field)
    {
        return std::nullopt;
    }

    if (std::holds_alternative<std::nullptr_t>(field->value))
    {
        return std::nullopt;
    }

    return asInt(*field, fieldName);
}

[[nodiscard]] std::optional<VegetationPlacementMode> getOptionalPlacementMode(
    const JsonObject& object,
    const std::string_view fieldName,
    const std::string_view context)
{
    const std::optional<std::string> value = getOptionalString(object, fieldName);

    if (!value.has_value())
    {
        return std::nullopt;
    }

    if (*value == "tree_sparse")
    {
        return VegetationPlacementMode::TreeSparse;
    }

    if (*value == "ground_dense")
    {
        return VegetationPlacementMode::GroundDense;
    }

    if (*value == "prop_sparse")
    {
        return VegetationPlacementMode::PropSparse;
    }

    throw std::runtime_error(
        "Unsupported vegetation placement mode in tileset catalog: " + std::string(context) + "." + std::string(fieldName));
}

[[nodiscard]] std::vector<std::string> parseTags(const JsonObject& object)
{
    const JsonValue* tagsValue = getOptionalField(object, "tags");

    if (!tagsValue)
    {
        return {};
    }

    std::vector<std::string> tags;

    for (const JsonValue& entry : asArray(*tagsValue, "tags"))
    {
        tags.push_back(asString(entry, "tags[]"));
    }

    return tags;
}

[[nodiscard]] std::vector<std::string> parseOptionalStringArray(
    const JsonObject& object,
    const std::string_view fieldName,
    const std::string_view context)
{
    const JsonValue* arrayValue = getOptionalField(object, fieldName);

    if (!arrayValue)
    {
        return {};
    }

    std::vector<std::string> entries;

    for (const JsonValue& entry : asArray(*arrayValue, context))
    {
        entries.push_back(asString(entry, std::string(context) + "[]"));
    }

    return entries;
}

[[nodiscard]] std::vector<std::pair<std::string, float>> parseOptionalFloatObject(
    const JsonObject& object,
    const std::string_view fieldName,
    const std::string_view context)
{
    const JsonValue* objectValue = getOptionalField(object, fieldName);

    if (!objectValue)
    {
        return {};
    }

    std::vector<std::pair<std::string, float>> entries;
    const JsonObject& values = asObject(*objectValue, context);
    entries.reserve(values.size());

    for (const auto& [key, value] : values)
    {
        entries.emplace_back(key, asFloat(value, std::string(context) + "." + key));
    }

    return entries;
}

[[nodiscard]] TilesetAssetAtlasEntry parseAtlas(const JsonObject& object)
{
    const JsonObject& atlas = asObject(getRequiredField(object, "atlas", "tile"), "tile.atlas");
    return {
        asInt(getRequiredField(atlas, "tileId", "tile.atlas"), "tile.atlas.tileId"),
        asInt(getRequiredField(atlas, "gid", "tile.atlas"), "tile.atlas.gid"),
        asInt(getRequiredField(atlas, "col", "tile.atlas"), "tile.atlas.col"),
        asInt(getRequiredField(atlas, "row", "tile.atlas"), "tile.atlas.row"),
        asInt(getRequiredField(atlas, "pixelX", "tile.atlas"), "tile.atlas.pixelX"),
        asInt(getRequiredField(atlas, "pixelY", "tile.atlas"), "tile.atlas.pixelY")};
}

[[nodiscard]] TilesetAssetTerrainData parseTerrainData(const JsonObject& object)
{
    const JsonObject& terrain = asObject(getRequiredField(object, "terrain", "tile"), "tile.terrain");
    TilesetAssetTerrainData data;
    data.id = asString(getRequiredField(terrain, "id", "tile.terrain"), "tile.terrain.id");
    data.classification = asString(getRequiredField(terrain, "class", "tile.terrain"), "tile.terrain.class");
    data.variant = getOptionalString(terrain, "variant");
    data.transitionTo = getOptionalString(terrain, "transitionTo");
    data.animationFrame = getOptionalInt(terrain, "animationFrame");

    const JsonValue* autotileValue = getOptionalField(terrain, "autotile");

    if (autotileValue && !std::holds_alternative<std::nullptr_t>(autotileValue->value))
    {
        const JsonObject& autotile = asObject(*autotileValue, "tile.terrain.autotile");
        data.autotile = TilesetAssetAutotile{
            asInt(getRequiredField(autotile, "patternX", "tile.terrain.autotile"), "tile.terrain.autotile.patternX"),
            asInt(getRequiredField(autotile, "patternY", "tile.terrain.autotile"), "tile.terrain.autotile.patternY"),
            asString(getRequiredField(autotile, "role", "tile.terrain.autotile"), "tile.terrain.autotile.role")};
    }

    return data;
}

[[nodiscard]] std::optional<TilesetAssetAutotile> parseOptionalAutotile(
    const JsonObject& object,
    const std::string_view fieldName,
    const std::string_view context)
{
    const JsonValue* autotileValue = getOptionalField(object, fieldName);

    if (!autotileValue || std::holds_alternative<std::nullptr_t>(autotileValue->value))
    {
        return std::nullopt;
    }

    const std::string autotileContext = std::string(context) + "." + std::string(fieldName);
    const JsonObject& autotile = asObject(*autotileValue, autotileContext);
    return TilesetAssetAutotile{
        asInt(getRequiredField(autotile, "patternX", autotileContext), autotileContext + ".patternX"),
        asInt(getRequiredField(autotile, "patternY", autotileContext), autotileContext + ".patternY"),
        asString(getRequiredField(autotile, "role", autotileContext), autotileContext + ".role")};
}

[[nodiscard]] TilesetAssetObjectData parseObjectData(const JsonObject& object)
{
    const JsonObject& objectData = asObject(getRequiredField(object, "object", "tile"), "tile.object");
    const JsonObject& offset = asObject(getRequiredField(objectData, "offset", "tile.object"), "tile.object.offset");
    return {
        asString(getRequiredField(objectData, "id", "tile.object"), "tile.object.id"),
        asString(getRequiredField(objectData, "role", "tile.object"), "tile.object.role"),
        asString(getRequiredField(objectData, "family", "tile.object"), "tile.object.family"),
        asInt(getRequiredField(offset, "x", "tile.object.offset"), "tile.object.offset.x"),
        asInt(getRequiredField(offset, "y", "tile.object.offset"), "tile.object.offset.y"),
        getOptionalPlacementMode(objectData, "placementMode", "tile.object"),
        parseOptionalStringArray(objectData, "placeOn", "tile.object.placeOn"),
        parseOptionalFloatObject(objectData, "biomes", "tile.object.biomes")};
}

[[nodiscard]] TilesetAssetOverlayData parseOverlayData(const JsonObject& object)
{
    const JsonObject& overlay = asObject(getRequiredField(object, "overlay", "tile"), "tile.overlay");
    TilesetAssetOverlayData data;
    data.id = asString(getRequiredField(overlay, "id", "tile.overlay"), "tile.overlay.id");
    data.overlayClass = asString(getRequiredField(overlay, "class", "tile.overlay"), "tile.overlay.class");
    data.variant = getOptionalString(overlay, "variant");
    data.onSurface = getOptionalString(overlay, "onSurface");
    data.animationFrame = getOptionalInt(overlay, "animationFrame");
    data.autotile = parseOptionalAutotile(overlay, "autotile", "tile.overlay");
    return data;
}

[[nodiscard]] TilesetAssetTile parseTile(const JsonValue& value)
{
    const JsonObject& object = asObject(value, "tile");
    const std::string& kindValue = asString(getRequiredField(object, "kind", "tile"), "tile.kind");

    TilesetAssetTile tile;
    tile.atlas = parseAtlas(object);
    tile.name = asString(getRequiredField(object, "name", "tile"), "tile.name");
    tile.tags = parseTags(object);
    tile.walkable = asBool(getRequiredField(object, "walkable", "tile"), "tile.walkable");
    tile.notes = asString(getRequiredField(object, "notes", "tile"), "tile.notes");

    if (kindValue == "terrain")
    {
        tile.kind = TilesetAssetTileKind::Terrain;
        tile.terrain = parseTerrainData(object);
        return tile;
    }

    if (kindValue == "object")
    {
        tile.kind = TilesetAssetTileKind::Object;
        tile.object = parseObjectData(object);
        return tile;
    }

    if (kindValue == "overlay")
    {
        tile.kind = TilesetAssetTileKind::Overlay;
        tile.overlay = parseOverlayData(object);
        return tile;
    }

    if (kindValue == "empty")
    {
        tile.kind = TilesetAssetTileKind::Empty;
        return tile;
    }

    throw std::runtime_error("Unknown tile kind in tileset catalog: " + kindValue);
}

[[nodiscard]] TilesetAssetDocument parseDocument(
    const std::filesystem::path& catalogPath,
    const std::filesystem::path& assetRoot)
{
    const JsonValue rootValue = JsonParser(readFileContents(catalogPath)).parse();
    const JsonObject& root = asObject(rootValue, "root");

    (void)asInt(getRequiredField(root, "schemaVersion", "root"), "root.schemaVersion");

    const JsonObject& tileset = asObject(getRequiredField(root, "tileset", "root"), "root.tileset");
    const JsonObject& source = asObject(getRequiredField(tileset, "source", "root.tileset"), "root.tileset.source");
    const JsonObject& runtime = asObject(getRequiredField(tileset, "runtime", "root.tileset"), "root.tileset.runtime");
    const JsonObject& grid = asObject(getRequiredField(tileset, "grid", "root.tileset"), "root.tileset.grid");

    TilesetAssetMetadata metadata;
    metadata.id = asString(getRequiredField(tileset, "id", "root.tileset"), "root.tileset.id");
    metadata.source.tsx = asString(getRequiredField(source, "tsx", "root.tileset.source"), "root.tileset.source.tsx");
    metadata.source.image =
        asString(getRequiredField(source, "image", "root.tileset.source"), "root.tileset.source.image");
    metadata.runtime.image =
        asString(getRequiredField(runtime, "image", "root.tileset.runtime"), "root.tileset.runtime.image");
    metadata.grid = {
        asInt(getRequiredField(grid, "tileSize", "root.tileset.grid"), "root.tileset.grid.tileSize"),
        asInt(getRequiredField(grid, "columns", "root.tileset.grid"), "root.tileset.grid.columns"),
        asInt(getRequiredField(grid, "rows", "root.tileset.grid"), "root.tileset.grid.rows"),
        asInt(getRequiredField(grid, "tileCount", "root.tileset.grid"), "root.tileset.grid.tileCount")};
    std::vector<TilesetAssetTile> tiles;

    for (const JsonValue& tileValue : asArray(getRequiredField(root, "tiles", "root"), "root.tiles"))
    {
        tiles.push_back(parseTile(tileValue));
    }

    const std::filesystem::path normalizedAssetRoot = assetRoot.lexically_normal();
    return {
        normalizedAssetRoot,
        catalogPath.lexically_normal(),
        (normalizedAssetRoot / metadata.runtime.image).lexically_normal(),
        std::move(metadata),
        std::move(tiles)};
}

} // namespace

TilesetAssetDocument TilesetAssetDocument::loadFromFile(const std::filesystem::path& catalogPath)
{
    return parseDocument(catalogPath, inferAssetRootFromCatalogPath(catalogPath));
}

TilesetAssetDocument TilesetAssetDocument::loadFromAssetRoot(
    const std::filesystem::path& assetRoot,
    const std::filesystem::path& catalogRelativePath)
{
    return parseDocument(assetRoot / catalogRelativePath, assetRoot);
}

TilesetAssetDocument::TilesetAssetDocument(
    std::filesystem::path assetRoot,
    std::filesystem::path catalogPath,
    std::filesystem::path resolvedImagePath,
    TilesetAssetMetadata tileset,
    std::vector<TilesetAssetTile> tiles)
    : m_assetRoot(std::move(assetRoot))
    , m_catalogPath(std::move(catalogPath))
    , m_resolvedImagePath(std::move(resolvedImagePath))
    , m_tileset(std::move(tileset))
    , m_tiles(std::move(tiles))
{
}

const std::filesystem::path& TilesetAssetDocument::getAssetRoot() const noexcept
{
    return m_assetRoot;
}

const std::filesystem::path& TilesetAssetDocument::getCatalogPath() const noexcept
{
    return m_catalogPath;
}

const std::filesystem::path& TilesetAssetDocument::getResolvedImagePath() const noexcept
{
    return m_resolvedImagePath;
}

const TilesetAssetMetadata& TilesetAssetDocument::getTileset() const noexcept
{
    return m_tileset;
}

const std::vector<TilesetAssetTile>& TilesetAssetDocument::getTiles() const noexcept
{
    return m_tiles;
}

} // namespace detail
} // namespace rpg
