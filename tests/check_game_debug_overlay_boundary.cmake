file(READ "${GAME_CPP}" GAME_CPP_CONTENT)

if(NOT GAME_CPP_CONTENT MATCHES "\"GameDebugOverlaySupport\\.hpp\"")
    message(FATAL_ERROR "Game.cpp does not include GameDebugOverlaySupport.hpp")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "detail::updateDebugOverlayRuntime")
    message(FATAL_ERROR "Game.cpp does not delegate debug overlay frame updates through shared support helpers")
endif()

if(NOT GAME_CPP_CONTENT MATCHES "detail::drawDebugOverlay")
    message(FATAL_ERROR "Game.cpp does not delegate debug overlay drawing through shared support helpers")
endif()

if(GAME_CPP_CONTENT MATCHES "loadDebugOverlayFont")
    message(FATAL_ERROR "Game.cpp still owns debug overlay font loading inline")
endif()

if(GAME_CPP_CONTENT MATCHES "debugOverlayFont")
    message(FATAL_ERROR "Game.cpp still owns debug overlay font state inline")
endif()

if(GAME_CPP_CONTENT MATCHES "frameRateAccumulatedSeconds")
    message(FATAL_ERROR "Game.cpp still owns debug overlay frame-rate accumulation state inline")
endif()

if(GAME_CPP_CONTENT MATCHES "frameRateSampleCount")
    message(FATAL_ERROR "Game.cpp still owns debug overlay frame-rate sample state inline")
endif()

if(GAME_CPP_CONTENT MATCHES "displayedFramesPerSecond")
    message(FATAL_ERROR "Game.cpp still owns debug overlay displayed frame rate state inline")
endif()

if(GAME_CPP_CONTENT MATCHES "sf::Text[ \t\r\n]+debugOverlayText")
    message(FATAL_ERROR "Game.cpp still constructs debug overlay text inline")
endif()

if(GAME_CPP_CONTENT MATCHES "sf::RectangleShape[ \t\r\n]+debugOverlayBackground")
    message(FATAL_ERROR "Game.cpp still constructs the debug overlay background inline")
endif()
