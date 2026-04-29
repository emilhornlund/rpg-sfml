function(_rpg_assert_exists path description)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Missing ${description}: ${path}")
    endif()
endfunction()

function(_rpg_add_imported_sfml_target target_name library_path include_dir)
    if(TARGET ${target_name})
        return()
    endif()

    add_library(${target_name} SHARED IMPORTED GLOBAL)
    set_target_properties(${target_name} PROPERTIES
        IMPORTED_LOCATION "${library_path}"
        INTERFACE_INCLUDE_DIRECTORIES "${include_dir}"
    )
endfunction()

function(_rpg_detect_sfml_toolchain out_toolchain out_default_lib_dir)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
       CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
       CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
        set(${out_toolchain} "linux-gcc-x64" PARENT_SCOPE)
        set(${out_default_lib_dir} "${PROJECT_SOURCE_DIR}/extlib/libs-linux-gcc/x64" PARENT_SCOPE)
        return()
    endif()

    message(FATAL_ERROR
        "Unsupported toolchain for vendored SFML: "
        "system='${CMAKE_SYSTEM_NAME}', compiler='${CMAKE_CXX_COMPILER_ID}', processor='${CMAKE_SYSTEM_PROCESSOR}'. "
        "Only Linux + GNU + x86_64 is implemented right now. "
        "Add another explicit if() case in cmake/VendoredSFML.cmake before using this toolchain.")
endfunction()

function(rpg_resolve_vendored_sfml out_include_dir out_lib_dir)
    _rpg_detect_sfml_toolchain(sfml_toolchain default_lib_dir)

    set(resolved_include_dir "${PROJECT_SOURCE_DIR}/extlib/include")
    if(DEFINED RPG_SFML_INCLUDE_DIR AND NOT RPG_SFML_INCLUDE_DIR STREQUAL "")
        set(resolved_include_dir "${RPG_SFML_INCLUDE_DIR}")
    endif()

    if(DEFINED RPG_SFML_LIB_DIR AND NOT RPG_SFML_LIB_DIR STREQUAL "")
        set(resolved_lib_dir "${RPG_SFML_LIB_DIR}")
    else()
        set(resolved_lib_dir "${default_lib_dir}")
    endif()

    if(NOT EXISTS "${resolved_include_dir}/SFML/Graphics.hpp")
        message(FATAL_ERROR
            "No vendored SFML headers found in '${resolved_include_dir}'. "
            "Expected extlib/include/SFML or override RPG_SFML_INCLUDE_DIR.")
    endif()

    if(NOT EXISTS "${resolved_lib_dir}")
        message(FATAL_ERROR
            "Vendored SFML library directory does not exist for toolchain '${sfml_toolchain}': '${resolved_lib_dir}'. "
            "Populate it with the matching shared libraries or override RPG_SFML_LIB_DIR.")
    endif()

    set(${out_include_dir} "${resolved_include_dir}" PARENT_SCOPE)
    set(${out_lib_dir} "${resolved_lib_dir}" PARENT_SCOPE)
endfunction()

function(rpg_add_vendored_sfml_targets include_dir lib_dir)
    _rpg_detect_sfml_toolchain(sfml_toolchain default_lib_dir)
    _rpg_assert_exists("${include_dir}/SFML/Graphics.hpp" "SFML graphics headers")

    if(sfml_toolchain STREQUAL "linux-gcc-x64")
        _rpg_assert_exists("${lib_dir}/libsfml-system.so" "SFML system shared library")
        _rpg_assert_exists("${lib_dir}/libsfml-window.so" "SFML window shared library")
        _rpg_assert_exists("${lib_dir}/libsfml-graphics.so" "SFML graphics shared library")
        _rpg_assert_exists("${lib_dir}/libsfml-audio.so" "SFML audio shared library")
        _rpg_assert_exists("${lib_dir}/libsfml-network.so" "SFML network shared library")

        _rpg_add_imported_sfml_target(SFML::System "${lib_dir}/libsfml-system.so" "${include_dir}")
        _rpg_add_imported_sfml_target(SFML::Window "${lib_dir}/libsfml-window.so" "${include_dir}")
        _rpg_add_imported_sfml_target(SFML::Graphics "${lib_dir}/libsfml-graphics.so" "${include_dir}")
        _rpg_add_imported_sfml_target(SFML::Audio "${lib_dir}/libsfml-audio.so" "${include_dir}")
        _rpg_add_imported_sfml_target(SFML::Network "${lib_dir}/libsfml-network.so" "${include_dir}")
    else()
        message(FATAL_ERROR
            "No imported-target implementation exists for toolchain '${sfml_toolchain}'. "
            "Add an explicit if() case in rpg_add_vendored_sfml_targets() before using it.")
    endif()

    set_target_properties(SFML::Window PROPERTIES
        INTERFACE_LINK_LIBRARIES "SFML::System"
    )
    set_target_properties(SFML::Graphics PROPERTIES
        INTERFACE_LINK_LIBRARIES "SFML::Window;SFML::System"
    )
    set_target_properties(SFML::Audio PROPERTIES
        INTERFACE_LINK_LIBRARIES "SFML::System"
    )
    set_target_properties(SFML::Network PROPERTIES
        INTERFACE_LINK_LIBRARIES "SFML::System"
    )
endfunction()
