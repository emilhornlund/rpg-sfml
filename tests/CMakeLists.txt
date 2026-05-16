include(VendoredSFML)

rpg_resolve_vendored_sfml(RPG_TEST_SFML_INCLUDE_DIR RPG_TEST_SFML_LIB_DIR)

function(rpg_add_test_executable target)
    set(options)
    set(oneValueArgs BUILD_RPATH TEST_ENVIRONMENT)
    set(multiValueArgs SOURCES INCLUDE_DIRS COMPILE_DEFINITIONS LINK_LIBRARIES TEST_ARGS)
    cmake_parse_arguments(RPG_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT RPG_TEST_SOURCES)
        message(FATAL_ERROR "rpg_add_test_executable(${target}) requires SOURCES")
    endif()

    set(rpg_test_include_dirs
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src/main"
    )
    if(RPG_TEST_INCLUDE_DIRS)
        set(rpg_test_include_dirs ${RPG_TEST_INCLUDE_DIRS})
    endif()

    add_executable(${target} ${RPG_TEST_SOURCES})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_include_directories(${target} PRIVATE ${rpg_test_include_dirs})

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    if(RPG_TEST_COMPILE_DEFINITIONS)
        target_compile_definitions(${target} PRIVATE ${RPG_TEST_COMPILE_DEFINITIONS})
    endif()

    if(RPG_TEST_LINK_LIBRARIES)
        target_link_libraries(${target} PRIVATE ${RPG_TEST_LINK_LIBRARIES})
    endif()

    if(RPG_TEST_BUILD_RPATH)
        set_target_properties(${target} PROPERTIES
            BUILD_RPATH "${RPG_TEST_BUILD_RPATH}"
        )
    endif()

    add_test(NAME ${target} COMMAND ${target} ${RPG_TEST_TEST_ARGS})

    if(RPG_TEST_TEST_ENVIRONMENT)
        set_tests_properties(${target} PROPERTIES
            ENVIRONMENT "${RPG_TEST_TEST_ENVIRONMENT}"
        )
    endif()
endfunction()

rpg_add_test_executable(game_runtime_support_tests
    SOURCES
        GameRuntimeSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameRuntimeSupport.cpp"
)
rpg_add_test_executable(player_occlusion_silhouette_tests
    SOURCES
        PlayerOcclusionSilhouetteTests.cpp
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(game_asset_support_tests
    SOURCES
        GameAssetSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/TerrainAutotileSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    TEST_ARGS
        "${PROJECT_SOURCE_DIR}/assets"
)
rpg_add_test_executable(game_debug_overlay_support_tests
    SOURCES
        GameDebugOverlaySupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameDebugOverlaySupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRuntimeSupport.cpp"
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ARGS
        "${PROJECT_SOURCE_DIR}/assets"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(game_event_support_tests
    SOURCES
        GameEventSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameEventSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRenderSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRuntimeSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(game_resource_bootstrap_support_tests
    SOURCES
        GameResourceBootstrapSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameResourceBootstrapSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TerrainAutotileSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ARGS
        "${PROJECT_SOURCE_DIR}/assets"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(game_occlusion_composite_support_tests
    SOURCES
        GameOcclusionCompositeSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameOcclusionCompositeSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRenderSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(game_scene_render_support_tests
    SOURCES
        GameSceneRenderSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameResourceBootstrapSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameSceneRenderSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRenderBatchSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRenderSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRuntimeSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TerrainAutotileSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ARGS
        "${PROJECT_SOURCE_DIR}/assets"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(game_render_plan_support_tests
    SOURCES
        GameRenderPlanSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameRenderPlanSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRuntimeSupport.cpp"
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(game_render_batch_support_tests
    SOURCES
        GameRenderBatchSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameRenderBatchSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRuntimeSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TerrainAutotileSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ARGS
        "${PROJECT_SOURCE_DIR}/assets"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(game_render_support_tests
    SOURCES
        GameRenderSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/GameRenderSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
    LINK_LIBRARIES
        SFML::Graphics
    BUILD_RPATH
        "${RPG_TEST_SFML_LIB_DIR}"
    TEST_ENVIRONMENT
        "LD_LIBRARY_PATH=${RPG_TEST_SFML_LIB_DIR}"
)
rpg_add_test_executable(gameplay_module_types_tests
    SOURCES
        GameplayModuleTypesTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRuntimeSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/OverworldRuntime.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/WorldContent.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
    COMPILE_DEFINITIONS
        RPG_DEFAULT_ASSET_ROOT_PATH="${PROJECT_SOURCE_DIR}/assets"
)
rpg_add_test_executable(overworld_runtime_tests
    SOURCES
        OverworldRuntimeTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/GameRuntimeSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/OverworldRuntime.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/WorldContent.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
    COMPILE_DEFINITIONS
        RPG_DEFAULT_ASSET_ROOT_PATH="${PROJECT_SOURCE_DIR}/assets"
)
rpg_add_test_executable(overworld_vertical_slice_tests
    SOURCES
        OverworldVerticalSliceTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/WorldContent.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
    INCLUDE_DIRS
        "${PROJECT_SOURCE_DIR}/include"
    COMPILE_DEFINITIONS
        RPG_DEFAULT_ASSET_ROOT_PATH="${PROJECT_SOURCE_DIR}/assets"
)
rpg_add_test_executable(world_terrain_generator_tests
    SOURCES
        WorldTerrainGeneratorTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/VegetationAtlasSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/WorldContent.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
    COMPILE_DEFINITIONS
        RPG_DEFAULT_ASSET_ROOT_PATH="${PROJECT_SOURCE_DIR}/assets"
)
rpg_add_test_executable(terrain_autotile_support_tests
    SOURCES
        TerrainAutotileSupportTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/TerrainAutotileSupport.cpp"
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
    TEST_ARGS
        "${PROJECT_SOURCE_DIR}/assets/output/catalogs/overworld-terrain-tileset-catalog.json"
)
rpg_add_test_executable(tileset_asset_loader_tests
    SOURCES
        TilesetAssetLoaderTests.cpp
        "${PROJECT_SOURCE_DIR}/src/main/TilesetAssetLoader.cpp"
    TEST_ARGS
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
    NAME game_debug_overlay_boundary_check
    COMMAND ${CMAKE_COMMAND}
        -DGAME_CPP=${PROJECT_SOURCE_DIR}/src/main/Game.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_game_debug_overlay_boundary.cmake"
)
add_test(
    NAME game_runtime_shell_boundary_check
    COMMAND ${CMAKE_COMMAND}
        -DGAME_CPP=${PROJECT_SOURCE_DIR}/src/main/Game.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_game_runtime_shell_boundary.cmake"
)
add_test(
    NAME game_render_plan_boundary_check
    COMMAND ${CMAKE_COMMAND}
        -DGAME_CPP=${PROJECT_SOURCE_DIR}/src/main/Game.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_game_render_plan_boundary.cmake"
)
add_test(
    NAME game_occlusion_composite_boundary_check
    COMMAND ${CMAKE_COMMAND}
        -DGAME_CPP=${PROJECT_SOURCE_DIR}/src/main/Game.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_game_occlusion_composite_boundary.cmake"
)
add_test(
    NAME game_scene_render_boundary_check
    COMMAND ${CMAKE_COMMAND}
        -DGAME_CPP=${PROJECT_SOURCE_DIR}/src/main/Game.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_game_scene_render_boundary.cmake"
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
        -DTERRAIN_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/output/tilesets/overworld-terrain-tileset.png
        -DVEGETATION_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/output/tilesets/overworld-vegetation-tileset.png
        -DTERRAIN_CATALOG_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/output/catalogs/overworld-terrain-tileset-catalog.json
        -DVEGETATION_CATALOG_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/output/catalogs/overworld-vegetation-tileset-catalog.json
        -DPLAYER_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/output/spritesheets/player-walking-spritesheet.png
        -DDEBUG_OVERLAY_FONT_ASSET_PATH=${PROJECT_BINARY_DIR}/src/main/assets/output/fonts/PixelOperator8.ttf
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_main_asset_staging.cmake"
)
