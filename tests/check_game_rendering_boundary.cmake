file(READ "${GAME_HPP}" GAME_HPP_CONTENT)
file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(GAME_HPP_CONTENT MATCHES "#include <SFML/")
    message(FATAL_ERROR "Game.hpp should not include SFML headers")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->overworldRuntime\\.update")
    message(FATAL_ERROR "Game.cpp does not advance the overworld runtime")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->overworldRuntime\\.getFrameState\\(\\)")
    message(FATAL_ERROR "Game.cpp does not read render state from the overworld runtime")
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

if(GAME_CPP_CONTENT MATCHES "getPlayerMarkerPlacement")
    message(FATAL_ERROR "Game.cpp should not derive player marker placement directly")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "playerMarker\\.setPosition\\(\\{"
    OR NOT GAME_CPP_CONTENT MATCHES "frameState\\.playerMarker\\.position\\.x"
    OR NOT GAME_CPP_CONTENT MATCHES "frameState\\.playerMarker\\.position\\.y")
    message(FATAL_ERROR "Game.cpp does not position the player marker from overworld frame state")
endif()

string(FIND "${GAME_CPP_CONTENT}" "m_impl->window.setView(view);" VIEW_INDEX)
string(FIND "${GAME_CPP_CONTENT}" "m_impl->window.draw(playerMarker);" PLAYER_MARKER_DRAW_INDEX)

if(VIEW_INDEX EQUAL -1 OR PLAYER_MARKER_DRAW_INDEX EQUAL -1 OR VIEW_INDEX GREATER PLAYER_MARKER_DRAW_INDEX)
    message(FATAL_ERROR "Game.cpp does not apply camera framing before drawing the player marker")
endif()
