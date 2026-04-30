include(VendoredSFML)

rpg_resolve_vendored_sfml(RPG_SFML_VENDOR_INCLUDE_DIR RPG_SFML_VENDOR_LIB_DIR)
rpg_add_vendored_sfml_targets("${RPG_SFML_VENDOR_INCLUDE_DIR}" "${RPG_SFML_VENDOR_LIB_DIR}")

add_executable(main
    Camera.cpp
    Game.cpp
    Player.cpp
    World.cpp
    WorldTerrainGenerator.cpp
    main.cpp
)

target_compile_features(main PRIVATE cxx_std_20)
target_include_directories(main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(main PRIVATE SFML::Graphics)

set_target_properties(main PROPERTIES
    BUILD_RPATH "${RPG_SFML_VENDOR_LIB_DIR}"
)
