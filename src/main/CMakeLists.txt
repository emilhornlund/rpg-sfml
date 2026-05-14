include(VendoredSFML)

rpg_resolve_vendored_sfml(RPG_SFML_VENDOR_INCLUDE_DIR RPG_SFML_VENDOR_LIB_DIR)
rpg_add_vendored_sfml_targets("${RPG_SFML_VENDOR_INCLUDE_DIR}" "${RPG_SFML_VENDOR_LIB_DIR}")

add_executable(main
    BiomeSampler.cpp
    Camera.cpp
    Game.cpp
    GameRenderBatchSupport.cpp
    GameRuntimeSupport.cpp
    OverworldRuntime.cpp
    Player.cpp
    TerrainAutotileSupport.cpp
    TilesetAssetLoader.cpp
    VegetationAtlasSupport.cpp
    World.cpp
    WorldContent.cpp
    WorldTerrainGenerator.cpp
    main.cpp
)

target_compile_features(main PRIVATE cxx_std_20)
target_include_directories(main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../include")
target_compile_definitions(main PRIVATE RPG_DEFAULT_ASSET_ROOT_PATH="$<TARGET_FILE_DIR:main>/assets")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(main PRIVATE SFML::Graphics)

set(RPG_MAIN_ASSET_SOURCE_DIR "${PROJECT_SOURCE_DIR}/assets")
set(RPG_MAIN_ASSET_OUTPUT_DIR "$<TARGET_FILE_DIR:main>/assets")
set(RPG_MAIN_CATALOG_SOURCE_DIR "${RPG_MAIN_ASSET_SOURCE_DIR}/output/catalogs")
set(RPG_MAIN_CATALOG_OUTPUT_DIR "${RPG_MAIN_ASSET_OUTPUT_DIR}/output/catalogs")
set(RPG_MAIN_FONT_SOURCE_DIR "${RPG_MAIN_ASSET_SOURCE_DIR}/output/fonts")
set(RPG_MAIN_FONT_OUTPUT_DIR "${RPG_MAIN_ASSET_OUTPUT_DIR}/output/fonts")

add_custom_command(TARGET main POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${RPG_MAIN_ASSET_OUTPUT_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${RPG_MAIN_CATALOG_OUTPUT_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${RPG_MAIN_FONT_OUTPUT_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${RPG_MAIN_ASSET_SOURCE_DIR}/overworld-terrain-tileset.png"
        "${RPG_MAIN_ASSET_SOURCE_DIR}/overworld-vegetation-tileset.png"
        "${RPG_MAIN_ASSET_SOURCE_DIR}/player-walking-spritesheet.png"
        "${RPG_MAIN_ASSET_OUTPUT_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${RPG_MAIN_CATALOG_SOURCE_DIR}/overworld-terrain-tileset-catalog.json"
        "${RPG_MAIN_CATALOG_SOURCE_DIR}/overworld-vegetation-tileset-catalog.json"
        "${RPG_MAIN_CATALOG_OUTPUT_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${RPG_MAIN_FONT_SOURCE_DIR}/PixelOperator8.ttf"
        "${RPG_MAIN_FONT_OUTPUT_DIR}"
)

set_target_properties(main PROPERTIES
    BUILD_RPATH "${RPG_SFML_VENDOR_LIB_DIR}"
)
