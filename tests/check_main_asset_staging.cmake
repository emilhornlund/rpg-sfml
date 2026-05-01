if(NOT EXISTS "${ASSET_PATH}")
    message(FATAL_ERROR "Main terrain tileset asset was not staged: ${ASSET_PATH}")
endif()

file(SIZE "${ASSET_PATH}" ASSET_SIZE)

if(ASSET_SIZE LESS_EQUAL 0)
    message(FATAL_ERROR "Main terrain tileset asset is empty: ${ASSET_PATH}")
endif()
