file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(NOT GAME_CPP_CONTENT MATCHES "\"GameOcclusionCompositeSupport\\.hpp\"")
    message(FATAL_ERROR "Game.cpp does not include GameOcclusionCompositeSupport.hpp")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "detail::drawPlayerOcclusionComposite\\(")
    message(FATAL_ERROR "Game.cpp does not delegate player occlusion compositing through shared support helpers")
endif()

if(GAME_CPP_CONTENT MATCHES "makeOcclusionRenderSurfaceSize")
    message(FATAL_ERROR "Game.cpp still derives occlusion working-surface sizes inline")
endif()

if(GAME_CPP_CONTENT MATCHES "ensureRenderTextureSize")
    message(FATAL_ERROR "Game.cpp still prepares occlusion render textures inline")
endif()

if(GAME_CPP_CONTENT MATCHES "configureOcclusionCompositeSprite")
    message(FATAL_ERROR "Game.cpp still configures the occlusion composite sprite inline")
endif()

if(GAME_CPP_CONTENT MATCHES "playerMaskTexture\\.clear")
    message(FATAL_ERROR "Game.cpp still clears the player occlusion mask inline")
endif()

if(GAME_CPP_CONTENT MATCHES "occluderMaskTexture\\.clear")
    message(FATAL_ERROR "Game.cpp still clears the occluder mask inline")
endif()
