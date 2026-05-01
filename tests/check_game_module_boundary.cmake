file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(NOT GAME_CPP_CONTENT MATCHES "#include <main/OverworldRuntime\\.hpp>")
    message(FATAL_ERROR "Game.cpp does not include main/OverworldRuntime.hpp")
endif()

if(GAME_CPP_CONTENT MATCHES "#include <main/World\\.hpp>")
    message(FATAL_ERROR "Game.cpp should not include main/World.hpp directly")
endif()

if(GAME_CPP_CONTENT MATCHES "#include <main/Player\\.hpp>")
    message(FATAL_ERROR "Game.cpp should not include main/Player.hpp directly")
endif()

if(GAME_CPP_CONTENT MATCHES "#include <main/Camera\\.hpp>")
    message(FATAL_ERROR "Game.cpp should not include main/Camera.hpp directly")
endif()

if(GAME_CPP_CONTENT MATCHES "struct[ \t\r\n]+OverworldRuntime[ \t\r\n]*\\{")
    message(FATAL_ERROR "Game.cpp still defines OverworldRuntime inline")
endif()

if(GAME_CPP_CONTENT MATCHES "m_impl->world\\.")
    message(FATAL_ERROR "Game.cpp should not access World directly")
endif()

if(GAME_CPP_CONTENT MATCHES "m_impl->player\\.")
    message(FATAL_ERROR "Game.cpp should not access Player directly")
endif()

if(GAME_CPP_CONTENT MATCHES "m_impl->camera\\.")
    message(FATAL_ERROR "Game.cpp should not access Camera directly")
endif()
