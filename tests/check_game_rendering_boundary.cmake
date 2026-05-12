file(READ "${GAME_HPP}" GAME_HPP_CONTENT)
file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(GAME_HPP_CONTENT MATCHES "#include <SFML/")
    message(FATAL_ERROR "Game.hpp should not include SFML headers")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->overworldRuntime\\.update")
    message(FATAL_ERROR "Game.cpp does not advance the overworld runtime")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->overworldRuntime\\.getRenderSnapshot\\(\\)")
    message(FATAL_ERROR "Game.cpp does not read a render snapshot from the overworld runtime")
endif()

if(GAME_CPP_CONTENT MATCHES "m_impl->world\\.")
    message(FATAL_ERROR "Game.cpp should not derive render state directly from World")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->window\\.setView")
    message(FATAL_ERROR "Game.cpp does not apply camera framing to the render window")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->window\\.draw")
    message(FATAL_ERROR "Game.cpp does not draw overworld content")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "sf::Texture")
    message(FATAL_ERROR "Game.cpp does not own an SFML texture for terrain rendering")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "loadFromFile")
    message(FATAL_ERROR "Game.cpp does not load the terrain tileset asset")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "buildTerrainVertexArray")
    message(FATAL_ERROR "Game.cpp does not build batched terrain geometry")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "buildTileGridVertexArray")
    message(FATAL_ERROR "Game.cpp does not build batched tile-grid geometry")
endif()

if(GAME_CPP_CONTENT MATCHES "m_impl->window\\.draw\\(tileSprite\\);")
    message(FATAL_ERROR "Game.cpp still submits terrain through per-tile sprite draws")
endif()

if(GAME_CPP_CONTENT MATCHES "m_impl->window\\.draw\\(gridSegment\\);")
    message(FATAL_ERROR "Game.cpp still submits tile-grid overlay through per-rectangle draws")
endif()

if(GAME_CPP_CONTENT MATCHES "setFillColor\\(getTileColor\\(visibleTile\\.tileType\\)\\)")
    message(FATAL_ERROR "Game.cpp still renders terrain tiles through per-tile fill colors")
endif()

if(GAME_CPP_CONTENT MATCHES "getPlayerMarkerPlacement")
    message(FATAL_ERROR "Game.cpp should not derive player marker placement directly")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "collectOverlapQualifiedFrontGeneratedContentIndices")
    message(FATAL_ERROR "Game.cpp does not derive overlap-qualified occlusion candidates")
endif()

if(GAME_CPP_CONTENT MATCHES "if \\(playerMarkerIt != renderSnapshot\\.markers\\.end\\(\\) && !frontOccluderIndices\\.empty\\(\\)\\)")
    message(FATAL_ERROR "Game.cpp still gates the occlusion pass on broad front-occluder presence")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "if \\(playerMarkerIt != renderSnapshot\\.markers\\.end\\(\\) && !overlapQualifiedOcclusionCandidateIndices\\.empty\\(\\)\\)")
    message(FATAL_ERROR "Game.cpp does not gate the occlusion pass on overlap-qualified occlusion candidates")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "for \\(const std::size_t index : overlapQualifiedOcclusionCandidateIndices\\)")
    message(FATAL_ERROR "Game.cpp does not render the occluder mask from overlap-qualified occlusion candidates")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "renderMarker\\.position\\.x"
    OR NOT GAME_CPP_CONTENT MATCHES "renderMarker\\.position\\.y"
    OR NOT GAME_CPP_CONTENT MATCHES "renderSnapshot\\.markers")
    message(FATAL_ERROR "Game.cpp does not position player rendering from the overworld render snapshot")
endif()

string(FIND "${GAME_CPP_CONTENT}" "m_impl->window.setView(view);" VIEW_INDEX)
string(FIND "${GAME_CPP_CONTENT}" "playerSprite.setTextureRect" PLAYER_RECT_INDEX)
string(FIND "${GAME_CPP_CONTENT}" "m_impl->window.draw(playerSprite);" PLAYER_DRAW_INDEX)

if(PLAYER_RECT_INDEX EQUAL -1)
    message(FATAL_ERROR "Game.cpp does not select player sprite frames from snapshot metadata")
endif()

if(VIEW_INDEX EQUAL -1 OR PLAYER_DRAW_INDEX EQUAL -1 OR VIEW_INDEX GREATER PLAYER_DRAW_INDEX)
    message(FATAL_ERROR "Game.cpp does not apply camera framing before drawing the player sprite")
endif()
