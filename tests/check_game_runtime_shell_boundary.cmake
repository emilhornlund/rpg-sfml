file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(NOT GAME_CPP_CONTENT MATCHES "\"GameResourceBootstrapSupport\\.hpp\"")
    message(FATAL_ERROR "Game.cpp does not include GameResourceBootstrapSupport.hpp")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "\"GameEventSupport\\.hpp\"")
    message(FATAL_ERROR "Game.cpp does not include GameEventSupport.hpp")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "detail::loadGameRenderResources\\(")
    message(FATAL_ERROR "Game.cpp does not delegate shell resource bootstrap through shared support helpers")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "detail::processGameEvents\\(")
    message(FATAL_ERROR "Game.cpp does not delegate SFML event polling through shared support helpers")
endif()

if(GAME_CPP_CONTENT MATCHES "loadTerrainTileset")
    message(FATAL_ERROR "Game.cpp still owns terrain tileset loading inline")
endif()

if(GAME_CPP_CONTENT MATCHES "loadTerrainTilesetMetadata")
    message(FATAL_ERROR "Game.cpp still owns terrain metadata loading inline")
endif()

if(GAME_CPP_CONTENT MATCHES "loadPlayerSpritesheet")
    message(FATAL_ERROR "Game.cpp still owns player spritesheet loading inline")
endif()

if(GAME_CPP_CONTENT MATCHES "loadVegetationTileset")
    message(FATAL_ERROR "Game.cpp still owns vegetation tileset loading inline")
endif()

if(GAME_CPP_CONTENT MATCHES "loadVegetationTilesetMetadata")
    message(FATAL_ERROR "Game.cpp still owns vegetation metadata loading inline")
endif()

if(GAME_CPP_CONTENT MATCHES "getDirectionalKey")
    message(FATAL_ERROR "Game.cpp still owns directional key translation inline")
endif()

if(GAME_CPP_CONTENT MATCHES "getDebugViewAction")
    message(FATAL_ERROR "Game.cpp still owns debug-view key translation inline")
endif()

if(GAME_CPP_CONTENT MATCHES "pollEvent")
    message(FATAL_ERROR "Game.cpp still polls SFML events inline")
endif()
