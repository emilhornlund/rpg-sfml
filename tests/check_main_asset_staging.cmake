if(NOT EXISTS "${TERRAIN_ASSET_PATH}")
    message(FATAL_ERROR "Main terrain tileset asset was not staged: ${TERRAIN_ASSET_PATH}")
endif()

if(NOT EXISTS "${VEGETATION_ASSET_PATH}")
    message(FATAL_ERROR "Main vegetation tileset asset was not staged: ${VEGETATION_ASSET_PATH}")
endif()

if(NOT EXISTS "${TERRAIN_CATALOG_ASSET_PATH}")
    message(FATAL_ERROR "Main terrain catalog asset was not staged: ${TERRAIN_CATALOG_ASSET_PATH}")
endif()

if(NOT EXISTS "${VEGETATION_CATALOG_ASSET_PATH}")
    message(FATAL_ERROR "Main vegetation catalog asset was not staged: ${VEGETATION_CATALOG_ASSET_PATH}")
endif()

if(NOT EXISTS "${PLAYER_ASSET_PATH}")
    message(FATAL_ERROR "Main player spritesheet asset was not staged: ${PLAYER_ASSET_PATH}")
endif()

if(NOT EXISTS "${DEBUG_OVERLAY_FONT_ASSET_PATH}")
    message(FATAL_ERROR "Main debug overlay font asset was not staged: ${DEBUG_OVERLAY_FONT_ASSET_PATH}")
endif()

file(SIZE "${TERRAIN_ASSET_PATH}" TERRAIN_ASSET_SIZE)
file(SIZE "${VEGETATION_ASSET_PATH}" VEGETATION_ASSET_SIZE)
file(SIZE "${TERRAIN_CATALOG_ASSET_PATH}" TERRAIN_CATALOG_ASSET_SIZE)
file(SIZE "${VEGETATION_CATALOG_ASSET_PATH}" VEGETATION_CATALOG_ASSET_SIZE)
file(SIZE "${PLAYER_ASSET_PATH}" PLAYER_ASSET_SIZE)
file(SIZE "${DEBUG_OVERLAY_FONT_ASSET_PATH}" DEBUG_OVERLAY_FONT_ASSET_SIZE)

if(TERRAIN_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main terrain tileset asset is empty: ${TERRAIN_ASSET_PATH}")
endif()

if(VEGETATION_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main vegetation tileset asset is empty: ${VEGETATION_ASSET_PATH}")
endif()

if(TERRAIN_CATALOG_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main terrain catalog asset is empty: ${TERRAIN_CATALOG_ASSET_PATH}")
endif()

if(VEGETATION_CATALOG_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main vegetation catalog asset is empty: ${VEGETATION_CATALOG_ASSET_PATH}")
endif()

if(PLAYER_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main player spritesheet asset is empty: ${PLAYER_ASSET_PATH}")
endif()

if(DEBUG_OVERLAY_FONT_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main debug overlay font asset is empty: ${DEBUG_OVERLAY_FONT_ASSET_PATH}")
endif()
