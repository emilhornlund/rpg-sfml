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
