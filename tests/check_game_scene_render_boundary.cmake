file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(NOT GAME_CPP_CONTENT MATCHES "\"GameSceneRenderSupport\\.hpp\"")
    message(FATAL_ERROR "Game.cpp does not include GameSceneRenderSupport.hpp")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "detail::buildOverworldSceneRenderFrame\\(")
    message(FATAL_ERROR "Game.cpp does not delegate overworld scene render-frame preparation")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "detail::renderOverworldScene\\(")
    message(FATAL_ERROR "Game.cpp does not delegate overworld scene pass orchestration")
endif()

if(GAME_CPP_CONTENT MATCHES "detail::buildTerrainVertexArray\\(")
    message(FATAL_ERROR "Game.cpp still prepares terrain batches inline")
endif()

if(GAME_CPP_CONTENT MATCHES "detail::buildTileGridVertexArray\\(")
    message(FATAL_ERROR "Game.cpp still prepares tile-grid batches inline")
endif()

if(GAME_CPP_CONTENT MATCHES "detail::executeOverworldRenderPasses\\(")
    message(FATAL_ERROR "Game.cpp still orchestrates overworld render passes inline")
endif()

if(GAME_CPP_CONTENT MATCHES "drawVegetationContent\\(")
    message(FATAL_ERROR "Game.cpp still draws generated content inline")
endif()

if(GAME_CPP_CONTENT MATCHES "detail::drawPlayerMarker\\(")
    message(FATAL_ERROR "Game.cpp still draws player markers inline")
endif()
