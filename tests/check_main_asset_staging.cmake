if(NOT EXISTS "${TERRAIN_ASSET_PATH}")
    message(FATAL_ERROR "Main terrain tileset asset was not staged: ${TERRAIN_ASSET_PATH}")
endif()

if(NOT EXISTS "${VEGETATION_ASSET_PATH}")
    message(FATAL_ERROR "Main vegetation tileset asset was not staged: ${VEGETATION_ASSET_PATH}")
endif()

if(NOT EXISTS "${TERRAIN_CLASSIFICATION_ASSET_PATH}")
    message(FATAL_ERROR "Main terrain classification asset was not staged: ${TERRAIN_CLASSIFICATION_ASSET_PATH}")
endif()

if(NOT EXISTS "${VEGETATION_CLASSIFICATION_ASSET_PATH}")
    message(FATAL_ERROR "Main vegetation classification asset was not staged: ${VEGETATION_CLASSIFICATION_ASSET_PATH}")
endif()

if(NOT EXISTS "${PLAYER_ASSET_PATH}")
    message(FATAL_ERROR "Main player spritesheet asset was not staged: ${PLAYER_ASSET_PATH}")
endif()

file(SIZE "${TERRAIN_ASSET_PATH}" TERRAIN_ASSET_SIZE)
file(SIZE "${VEGETATION_ASSET_PATH}" VEGETATION_ASSET_SIZE)
file(SIZE "${TERRAIN_CLASSIFICATION_ASSET_PATH}" TERRAIN_CLASSIFICATION_ASSET_SIZE)
file(SIZE "${VEGETATION_CLASSIFICATION_ASSET_PATH}" VEGETATION_CLASSIFICATION_ASSET_SIZE)
file(SIZE "${PLAYER_ASSET_PATH}" PLAYER_ASSET_SIZE)

if(TERRAIN_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main terrain tileset asset is empty: ${TERRAIN_ASSET_PATH}")
endif()

if(VEGETATION_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main vegetation tileset asset is empty: ${VEGETATION_ASSET_PATH}")
endif()

if(TERRAIN_CLASSIFICATION_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main terrain classification asset is empty: ${TERRAIN_CLASSIFICATION_ASSET_PATH}")
endif()

if(VEGETATION_CLASSIFICATION_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main vegetation classification asset is empty: ${VEGETATION_CLASSIFICATION_ASSET_PATH}")
endif()

if(PLAYER_ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main player spritesheet asset is empty: ${PLAYER_ASSET_PATH}")
endif()
