file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(NOT GAME_CPP_CONTENT MATCHES "\"GameRenderPlanSupport\\.hpp\"")
    message(FATAL_ERROR "Game.cpp does not include GameRenderPlanSupport.hpp")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "detail::buildOverworldRenderPlan\\(")
    message(FATAL_ERROR "Game.cpp does not delegate render-plan assembly through shared support helpers")
endif()

if(GAME_CPP_CONTENT MATCHES "std::stable_sort")
    message(FATAL_ERROR "Game.cpp still sorts the render queue inline")
endif()

if(GAME_CPP_CONTENT MATCHES "collectFrontGeneratedContentIndices")
    message(FATAL_ERROR "Game.cpp still derives front occluders inline")
endif()

if(GAME_CPP_CONTENT MATCHES "collectOverlapQualifiedFrontGeneratedContentIndices")
    message(FATAL_ERROR "Game.cpp still derives overlap-qualified occlusion candidates inline")
endif()

if(GAME_CPP_CONTENT MATCHES "std::find_if")
    message(FATAL_ERROR "Game.cpp still looks up the player marker inline as part of render-plan assembly")
endif()
