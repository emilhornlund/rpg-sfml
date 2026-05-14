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

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->overworldRuntime\\.getDebugSnapshot\\(\\)")
    message(FATAL_ERROR "Game.cpp does not read a debug snapshot from the overworld runtime")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->window\\.clear")
    message(FATAL_ERROR "Game.cpp does not begin frame rendering through a shell-owned window clear")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "m_impl->window\\.display")
    message(FATAL_ERROR "Game.cpp does not present the rendered frame through the shell-owned window")
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
