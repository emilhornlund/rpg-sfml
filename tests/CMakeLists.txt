add_executable(game_runtime_support_tests
    GameRuntimeSupportTests.cpp
)

target_compile_features(game_runtime_support_tests PRIVATE cxx_std_20)
target_include_directories(game_runtime_support_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/src/main"
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(game_runtime_support_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME game_runtime_support_tests COMMAND game_runtime_support_tests)
add_test(
    NAME main_handoff_check
    COMMAND ${CMAKE_COMMAND}
        -DMAIN_CPP=${PROJECT_SOURCE_DIR}/src/main/main.cpp
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_main_handoff.cmake"
)
