file(READ "${MAIN_CPP}" MAIN_CPP_CONTENT)

if(NOT MAIN_CPP_CONTENT MATCHES "#include <main/Game\\.hpp>")
    message(FATAL_ERROR "main.cpp does not include main/Game.hpp")
endif()

if(NOT MAIN_CPP_CONTENT MATCHES "rpg::Game[ \t\r\n]+game;")
    message(FATAL_ERROR "main.cpp does not construct rpg::Game")
endif()

if(NOT MAIN_CPP_CONTENT MATCHES "return[ \t\r\n]+game\\.run\\(\\);")
    message(FATAL_ERROR "main.cpp does not hand off to game.run()")
endif()
