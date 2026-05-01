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

if(GAME_CPP_CONTENT MATCHES "getPlayerMarkerPlacement")
    message(FATAL_ERROR "Game.cpp should not derive player marker placement directly")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "renderMarker\\.position\\.x"
    OR NOT GAME_CPP_CONTENT MATCHES "renderMarker\\.position\\.y"
    OR NOT GAME_CPP_CONTENT MATCHES "renderSnapshot\\.markers")
    message(FATAL_ERROR "Game.cpp does not position render markers from the overworld render snapshot")
endif()

string(FIND "${GAME_CPP_CONTENT}" "m_impl->window.setView(view);" VIEW_INDEX)
string(FIND "${GAME_CPP_CONTENT}" "m_impl->window.draw(markerShape);" MARKER_DRAW_INDEX)

if(VIEW_INDEX EQUAL -1 OR MARKER_DRAW_INDEX EQUAL -1 OR VIEW_INDEX GREATER MARKER_DRAW_INDEX)
    message(FATAL_ERROR "Game.cpp does not apply camera framing before drawing snapshot markers")
endif()
