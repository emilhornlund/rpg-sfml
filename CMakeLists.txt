cmake_minimum_required(VERSION 3.28)
project(RPG_SFML LANGUAGES CXX)

# Basics
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Warnings (cross-platform)
if (MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

add_subdirectory(src/Game)

set(CLANG_FORMAT_EXECUTABLE "clang-format" CACHE STRING "Path or name of clang-format executable")
add_custom_target(
        rpg-tools-format
        COMMAND ${CMAKE_COMMAND} -DCLANG_FORMAT_EXECUTABLE=${CLANG_FORMAT_EXECUTABLE}
        -P ${PROJECT_SOURCE_DIR}/cmake/Format.cmake
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Formatting sources with clang-format"
        VERBATIM
)

add_custom_target(
        rpg-tools-format-check
        COMMAND ${CMAKE_COMMAND}
        -DCLANG_FORMAT_EXECUTABLE=${CLANG_FORMAT_EXECUTABLE}
        -P ${PROJECT_SOURCE_DIR}/cmake/FormatCheck.cmake
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Checking code style with clang-format (no changes)"
        VERBATIM
)
