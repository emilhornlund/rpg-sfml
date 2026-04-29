file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(NOT GAME_CPP_CONTENT MATCHES "#include <main/World\\.hpp>")
    message(FATAL_ERROR "Game.cpp does not include main/World.hpp")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "#include <main/Player\\.hpp>")
    message(FATAL_ERROR "Game.cpp does not include main/Player.hpp")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "#include <main/Camera\\.hpp>")
    message(FATAL_ERROR "Game.cpp does not include main/Camera.hpp")
endif()

if(GAME_CPP_CONTENT MATCHES "struct[ \t\r\n]+World[ \t\r\n]*\\{")
    message(FATAL_ERROR "Game.cpp still defines World inline")
endif()

if(GAME_CPP_CONTENT MATCHES "struct[ \t\r\n]+Player[ \t\r\n]*\\{")
    message(FATAL_ERROR "Game.cpp still defines Player inline")
endif()

if(GAME_CPP_CONTENT MATCHES "struct[ \t\r\n]+Camera[ \t\r\n]*\\{")
    message(FATAL_ERROR "Game.cpp still defines Camera inline")
endif()
