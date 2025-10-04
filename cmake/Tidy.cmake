if(NOT DEFINED CLANG_TIDY_EXECUTABLE)
    set(CLANG_TIDY_EXECUTABLE "clang-tidy")
endif()

if(NOT EXISTS "${CLANG_TIDY_EXECUTABLE}")
    find_program(CLANG_TIDY_EXEC_TEMP NAMES "${CLANG_TIDY_EXECUTABLE}" clang-tidy clang-tidy-20 clang-tidy-19 clang-tidy-18)
    if(CLANG_TIDY_EXEC_TEMP)
        set(CLANG_TIDY_EXECUTABLE "${CLANG_TIDY_EXEC_TEMP}")
        unset(CLANG_TIDY_EXEC_TEMP)
    else()
        message(FATAL_ERROR "Unable to find clang-tidy executable: \"${CLANG_TIDY_EXECUTABLE}\"")
    endif()
endif()

execute_process(COMMAND "${CLANG_TIDY_EXECUTABLE}" --version OUTPUT_VARIABLE _CT_VER_RAW)
string(REGEX MATCH "version ([0-9]+)" _CT_VER_MATCH "${_CT_VER_RAW}")
if(NOT _CT_VER_MATCH)
    message(FATAL_ERROR "Failed to parse clang-tidy version from: ${_CT_VER_RAW}")
endif()
set(_CT_VER_MAJOR "${CMAKE_MATCH_1}")
if(_CT_VER_MAJOR LESS 14)
    message(FATAL_ERROR "clang-tidy version ${_CT_VER_MAJOR} is too low; need >= 14")
else()
    message(STATUS "Using clang-tidy version ${_CT_VER_MAJOR}")
endif()

if(NOT DEFINED PROJECT_BINARY_DIR)
    message(FATAL_ERROR "PROJECT_BINARY_DIR not set (pass -DPROJECT_BINARY_DIR=${CMAKE_BINARY_DIR})")
endif()
if(NOT EXISTS "${PROJECT_BINARY_DIR}/compile_commands.json")
    message(FATAL_ERROR "Missing compile_commands.json. Enable with: set(CMAKE_EXPORT_COMPILE_COMMANDS ON)")
endif()

set(_EXTRA_TIDY_ARGS "")
find_program(CLANG_BIN NAMES clang clang-20 clang-19 clang-18 clang-17 clang-16 clang-15 clang-14)
if(CLANG_BIN)
    execute_process(
            COMMAND "${CLANG_BIN}" -print-resource-dir
            OUTPUT_VARIABLE _CLANG_RESOURCE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE _RES
    )
    if(_RES EQUAL 0 AND EXISTS "${_CLANG_RESOURCE_DIR}")
        list(APPEND _EXTRA_TIDY_ARGS "-extra-arg=-resource-dir=${_CLANG_RESOURCE_DIR}")
        message(STATUS "clang resource-dir: ${_CLANG_RESOURCE_DIR}")
    else()
        message(WARNING "Unable to determine clang resource-dir from: ${CLANG_BIN}")
    endif()
else()
    message(WARNING "No 'clang' binary found to query resource-dir; proceeding without it.")
endif()

set(_HEADER_FILTER_REGEX ".*/(include|src)/.*")

set(SOURCES "")
foreach(_d IN ITEMS include src)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${_d}")
        file(GLOB_RECURSE _FILES
                "${_d}/*.c"  "${_d}/*.cc"  "${_d}/*.cpp"  "${_d}/*.cxx"
                "${_d}/*.h"  "${_d}/*.hh"  "${_d}/*.hpp"  "${_d}/*.hxx"
                "${_d}/*.inl"
        )
        list(APPEND SOURCES ${_FILES})
    endif()
endforeach()

list(FILTER SOURCES EXCLUDE REGEX ".*/extlibs/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/_deps/.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/third[_-]?party/.*")

if(SOURCES STREQUAL "")
    message(STATUS "No sources found for clang-tidy.")
    return()
endif()

if(DEFINED ENV{NUMBER_OF_PROCESSORS} AND NOT "$ENV{NUMBER_OF_PROCESSORS}" STREQUAL "")
    set(_PARALLEL_J "$ENV{NUMBER_OF_PROCESSORS}")
else()
    include(ProcessorCount)
    ProcessorCount(_CPU_COUNT)
    if(NOT _CPU_COUNT OR _CPU_COUNT EQUAL 0)
        set(_PARALLEL_J 1)
    else()
        set(_PARALLEL_J "${_CPU_COUNT}")
    endif()
endif()

find_program(RUN_CLANG_TIDY NAMES run-clang-tidy run-clang-tidy.py)
if(RUN_CLANG_TIDY)
    message(STATUS "Using helper: ${RUN_CLANG_TIDY}")
    execute_process(
            COMMAND "${RUN_CLANG_TIDY}"
            -j "${_PARALLEL_J}"
            -clang-tidy-binary "${CLANG_TIDY_EXECUTABLE}"
            -p "${PROJECT_BINARY_DIR}"
            "-header-filter=${_HEADER_FILTER_REGEX}"
            ${_EXTRA_TIDY_ARGS}
            -use-color=1
            -quiet
            ${SOURCES}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE _RES
    )
    if(NOT _RES EQUAL 0)
        message(FATAL_ERROR "clang-tidy analysis failed (run-clang-tidy).")
    endif()

else()
    message(WARNING "run-clang-tidy not found; falling back to per-file invocation.")
    set(_FAILED FALSE)
    foreach(_f IN LISTS SOURCES)
        execute_process(
                COMMAND "${CLANG_TIDY_EXECUTABLE}"
                -p "${PROJECT_BINARY_DIR}"
                "-header-filter=${_HEADER_FILTER_REGEX}"
                ${_EXTRA_TIDY_ARGS}
                -use-color=1
                -quiet
                "${_f}"
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE _RES
                OUTPUT_VARIABLE _OUT
                ERROR_VARIABLE  _ERR
        )
        if(NOT _RES EQUAL 0)
            set(_FAILED TRUE)
            message(STATUS "clang-tidy failed: ${_f}")
            if(_OUT)
                message(STATUS "${_OUT}")
            endif()
            if(_ERR)
                message(STATUS "${_ERR}")
            endif()
        endif()
    endforeach()
    if(_FAILED)
        message(FATAL_ERROR "clang-tidy analysis reported issues.")
    endif()
endif()
