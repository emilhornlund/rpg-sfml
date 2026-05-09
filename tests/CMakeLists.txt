add_executable(game_runtime_support_tests
    GameRuntimeSupportTests.cpp
)
add_executable(game_asset_support_tests
    GameAssetSupportTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/TerrainAutotileSupport.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
)

add_executable(gameplay_module_types_tests
    GameplayModuleTypesTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/OverworldRuntime.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldContent.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
)
add_executable(overworld_runtime_tests
    OverworldRuntimeTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/OverworldRuntime.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldContent.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
)
add_executable(overworld_vertical_slice_tests
    OverworldVerticalSliceTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldContent.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
)
add_executable(world_terrain_generator_tests
    WorldTerrainGeneratorTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldContent.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
)
add_executable(terrain_autotile_support_tests
    TerrainAutotileSupportTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/TerrainAutotileSupport.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
)
add_executable(tileset_asset_loader_tests
    TilesetAssetLoaderTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
)

target_compile_features(game_runtime_support_tests PRIVATE cxx_std_20)
target_compile_features(game_asset_support_tests PRIVATE cxx_std_20)
target_compile_features(gameplay_module_types_tests PRIVATE cxx_std_20)
target_compile_features(overworld_runtime_tests PRIVATE cxx_std_20)
target_compile_features(overworld_vertical_slice_tests PRIVATE cxx_std_20)
target_compile_features(world_terrain_generator_tests PRIVATE cxx_std_20)
target_compile_features(terrain_autotile_support_tests PRIVATE cxx_std_20)
target_compile_features(tileset_asset_loader_tests PRIVATE cxx_std_20)

target_include_directories(game_runtime_support_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src/main"
)
target_include_directories(game_asset_support_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src/main"
)
target_include_directories(gameplay_module_types_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src/main"
)
target_include_directories(overworld_runtime_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src/main"
)
target_include_directories(overworld_vertical_slice_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
)
target_include_directories(world_terrain_generator_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src/main"
)
target_include_directories(terrain_autotile_support_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src/main"
)
target_include_directories(tileset_asset_loader_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src/main"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(game_runtime_support_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(game_asset_support_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(gameplay_module_types_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(overworld_runtime_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(overworld_vertical_slice_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(world_terrain_generator_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(terrain_autotile_support_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(tileset_asset_loader_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(gameplay_module_types_tests PRIVATE RPG_DEFAULT_ASSET_ROOT_PATH="${PROJECT_SOURCE_DIR}/assets")
target_compile_definitions(overworld_runtime_tests PRIVATE RPG_DEFAULT_ASSET_ROOT_PATH="${PROJECT_SOURCE_DIR}/assets")
target_compile_definitions(overworld_vertical_slice_tests PRIVATE RPG_DEFAULT_ASSET_ROOT_PATH="${PROJECT_SOURCE_DIR}/assets")
target_compile_definitions(world_terrain_generator_tests PRIVATE RPG_DEFAULT_ASSET_ROOT_PATH="${PROJECT_SOURCE_DIR}/assets")

add_test(NAME game_runtime_support_tests COMMAND game_runtime_support_tests)
add_test(
    NAME game_asset_support_tests
    COMMAND game_asset_support_tests
        "${PROJECT_SOURCE_DIR}/assets"
)
add_test(NAME gameplay_module_types_tests COMMAND gameplay_module_types_tests)
add_test(NAME overworld_runtime_tests COMMAND overworld_runtime_tests)
add_test(NAME overworld_vertical_slice_tests COMMAND overworld_vertical_slice_tests)
add_test(NAME world_terrain_generator_tests COMMAND world_terrain_generator_tests)
add_test(
    NAME terrain_autotile_support_tests
    COMMAND terrain_autotile_support_tests
        "${PROJECT_SOURCE_DIR}/assets/output/classifications/overworld-terrain-tileset-classification.json"
)
add_test(
    NAME tileset_asset_loader_tests
    COMMAND tileset_asset_loader_tests
        "${PROJECT_SOURCE_DIR}/assets"
)
add_test(
    NAME main_handoff_check
    COMMAND ${CMAKE_COMMAND}
        -DMAIN_CPP=${PROJECT_SOURCE_DIR}/src/main/main.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_main_handoff.cmake"
)
add_test(
    NAME game_module_boundary_check
    COMMAND ${CMAKE_COMMAND}
        -DGAME_CPP=${PROJECT_SOURCE_DIR}/src/main/Game.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_game_module_boundary.cmake"
)
add_test(
    NAME game_rendering_boundary_check
    COMMAND ${CMAKE_COMMAND}
        -DGAME_HPP=${PROJECT_SOURCE_DIR}/include/main/Game.hpp
        -DGAME_CPP=${PROJECT_SOURCE_DIR}/src/main/Game.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_game_rendering_boundary.cmake"
)
add_test(
    NAME main_asset_staging_check
    COMMAND ${CMAKE_COMMAND}
        -DTERRAIN_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/overworld-terrain-tileset.png
        -DVEGETATION_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/overworld-vegetation-tileset.png
        -DTERRAIN_CLASSIFICATION_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/output/classifications/overworld-terrain-tileset-classification.json
        -DVEGETATION_CLASSIFICATION_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/output/classifications/overworld-vegetation-tileset-classification.json
        -DPLAYER_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/player-walking-spritesheet.png
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_main_asset_staging.cmake"
)
