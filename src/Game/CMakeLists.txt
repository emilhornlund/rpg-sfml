set(TARGET rpg)

set(INCROOT ${PROJECT_SOURCE_DIR}/include/RPG/Game)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Game)

set(SRC
    ${INCROOT}/Game.hpp
    ${SRCROOT}/Game.cpp
    ${SRCROOT}/main.cpp
)

add_executable(${TARGET} ${SRC})

target_link_libraries(${TARGET} PRIVATE SFML::Graphics)

target_include_directories(${TARGET} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

target_compile_features(${TARGET} PRIVATE cxx_std_20)

if (NOT MSVC)
    target_compile_options(${TARGET} PRIVATE -Wconversion -Wsign-conversion)
endif()

target_compile_options(${TARGET} PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fsanitize=address,undefined>
)

target_link_options(${TARGET} PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fsanitize=address,undefined>
)

target_compile_options(${TARGET} PRIVATE
        $<$<AND:$<CONFIG:Debug>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-fno-omit-frame-pointer>
)
