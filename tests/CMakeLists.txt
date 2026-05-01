add_executable(game_runtime_support_tests
    GameRuntimeSupportTests.cpp
)

add_executable(gameplay_module_types_tests
    GameplayModuleTypesTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/OverworldRuntime.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
)
add_executable(overworld_runtime_tests
    OverworldRuntimeTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/OverworldRuntime.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
)
add_executable(overworld_vertical_slice_tests
    OverworldVerticalSliceTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Camera.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/Player.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
)
add_executable(world_terrain_generator_tests
    WorldTerrainGeneratorTests.cpp
    "${PROJECT_SOURCE_DIR}/src/main/BiomeSampler.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/World.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorldTerrainGenerator.cpp"
)

target_compile_features(game_runtime_support_tests PRIVATE cxx_std_20)
target_compile_features(gameplay_module_types_tests PRIVATE cxx_std_20)
target_compile_features(overworld_runtime_tests PRIVATE cxx_std_20)
target_compile_features(overworld_vertical_slice_tests PRIVATE cxx_std_20)
target_compile_features(world_terrain_generator_tests PRIVATE cxx_std_20)

target_include_directories(game_runtime_support_tests PRIVATE
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

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(game_runtime_support_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(gameplay_module_types_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(overworld_runtime_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(overworld_vertical_slice_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(world_terrain_generator_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME game_runtime_support_tests COMMAND game_runtime_support_tests)
add_test(NAME gameplay_module_types_tests COMMAND gameplay_module_types_tests)
add_test(NAME overworld_runtime_tests COMMAND overworld_runtime_tests)
add_test(NAME overworld_vertical_slice_tests COMMAND overworld_vertical_slice_tests)
add_test(NAME world_terrain_generator_tests COMMAND world_terrain_generator_tests)
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
