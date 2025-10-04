# Check executable exists
if(NOT EXISTS ${CLANG_FORMAT_EXECUTABLE})
    find_program(CLANG_FORMAT_EXEC_TEMP ${CLANG_FORMAT_EXECUTABLE})
    if(CLANG_FORMAT_EXEC_TEMP)
        set(CLANG_FORMAT_EXECUTABLE ${CLANG_FORMAT_EXEC_TEMP})
        unset(CLANG_FORMAT_EXEC_TEMP)
    else()
        message(FATAL_ERROR "Unable to find clang-format executable: \"${CLANG_FORMAT_EXECUTABLE}\"")
    endif()
endif()

# Check executable version
execute_process(COMMAND "${CLANG_FORMAT_EXECUTABLE}" --version OUTPUT_VARIABLE _CF_VER_RAW)
string(REGEX MATCH "clang-format version ([0-9]+)" _CF_VER_MATCH "${_CF_VER_RAW}")
if(NOT _CF_VER_MATCH)
    message(FATAL_ERROR "Failed to parse clang-format version from: ${_CF_VER_RAW}")
endif()
set(_CF_VER_MAJOR "${CMAKE_MATCH_1}")

# Require >= 12 (adjust if you want stricter)
if(_CF_VER_MAJOR LESS 12)
    message(FATAL_ERROR "clang-format version ${_CF_VER_MAJOR} is too low; need >= 12")
else()
    message(STATUS "Using clang-format version ${_CF_VER_MAJOR}")
endif()

unset(_CF_VER_RAW)
unset(_CF_VER_MATCH)

# Run
set(SOURCES "")
foreach(FOLDER IN ITEMS include src)
    file(GLOB_RECURSE folder_files "${FOLDER}/*.h" "${FOLDER}/*.hpp" "${FOLDER}/*.inl" "${FOLDER}/*.cpp" "${FOLDER}/*.mm" "${FOLDER}/*.m")
    list(FILTER folder_files EXCLUDE REGEX "gl.h|vulkan.h|stb_perlin.h") # 3rd party code to exclude from formatting
    list(APPEND SOURCES ${folder_files})
endforeach()

execute_process(COMMAND "${CLANG_FORMAT_EXECUTABLE}" -i ${SOURCES})

message(STATUS "Formatting ${SOURCES}")
