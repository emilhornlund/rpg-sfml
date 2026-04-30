file(READ "${GAME_HPP}" GAME_HPP_CONTENT)
file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(GAME_HPP_CONTENT MATCHES "#include <SFML/")
    message(FATAL_ERROR "Game.hpp should not include SFML headers")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "initializeOverworldSlice\\(\\)")
    message(FATAL_ERROR "Game.cpp does not initialize the overworld slice")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->player\\.update")
    message(FATAL_ERROR "Game.cpp does not update the player module")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->camera\\.update")
    message(FATAL_ERROR "Game.cpp does not update the camera module")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->window\\.setView")
    message(FATAL_ERROR "Game.cpp does not apply camera framing to the render window")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->window\\.draw")
    message(FATAL_ERROR "Game.cpp does not draw overworld content")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "getPlayerMarkerPlacement")
    message(FATAL_ERROR "Game.cpp does not derive player marker placement through runtime support")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "playerMarker\\.setPosition\\(\\{"
    OR NOT GAME_CPP_CONTENT MATCHES "playerMarkerPlacement\\.position\\.x"
    OR NOT GAME_CPP_CONTENT MATCHES "playerMarkerPlacement\\.position\\.y")
    message(FATAL_ERROR "Game.cpp does not position the player marker from the current player position")
endif()

string(FIND "${GAME_CPP_CONTENT}" "m_impl->window.setView(view);" VIEW_INDEX)
string(FIND "${GAME_CPP_CONTENT}" "m_impl->window.draw(playerMarker);" PLAYER_MARKER_DRAW_INDEX)

if(VIEW_INDEX EQUAL -1 OR PLAYER_MARKER_DRAW_INDEX EQUAL -1 OR VIEW_INDEX GREATER PLAYER_MARKER_DRAW_INDEX)
    message(FATAL_ERROR "Game.cpp does not apply camera framing before drawing the player marker")
endif()
