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

# App
add_executable(rpg src/main.cpp)
target_link_libraries(rpg PRIVATE SFML::Graphics)
target_compile_features(rpg PRIVATE cxx_std_20)

if (NOT MSVC)
    target_compile_options(rpg PRIVATE -Wconversion -Wsign-conversion)
endif()

# Dev sanitizers on Unix Debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    target_compile_options(rpg PRIVATE -fsanitize=address,undefined)
    target_link_options(rpg PRIVATE -fsanitize=address,undefined)
endif()
