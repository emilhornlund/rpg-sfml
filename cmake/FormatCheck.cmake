# Ensure clang-format is resolvable the same way
if(NOT EXISTS ${CLANG_FORMAT_EXECUTABLE})
    find_program(CLANG_FORMAT_EXEC_TEMP ${CLANG_FORMAT_EXECUTABLE})
    if(CLANG_FORMAT_EXEC_TEMP)
        set(CLANG_FORMAT_EXECUTABLE ${CLANG_FORMAT_EXEC_TEMP})
        unset(CLANG_FORMAT_EXEC_TEMP)
    else()
        message(FATAL_ERROR "Unable to find clang-format executable: \"${CLANG_FORMAT_EXECUTABLE}\"")
    endif()
endif()

set(SOURCES "")
foreach(FOLDER IN ITEMS include src)
    file(GLOB_RECURSE folder_files
            "${FOLDER}/*.h" "${FOLDER}/*.hpp" "${FOLDER}/*.inl"
            "${FOLDER}/*.cpp" "${FOLDER}/*.mm" "${FOLDER}/*.m")
    list(FILTER folder_files EXCLUDE REGEX "gl.h|vulkan.h|stb_perlin.h")
    list(APPEND SOURCES ${folder_files})
endforeach()

# -n: dry-run, --Werror: treat diffs as errors
execute_process(
        COMMAND "${CLANG_FORMAT_EXECUTABLE}" --Werror -n ${SOURCES}
        RESULT_VARIABLE _CF_RES
        OUTPUT_VARIABLE _CF_OUT
        ERROR_VARIABLE  _CF_ERR
)
if(NOT _CF_RES EQUAL 0)
    message(FATAL_ERROR "clang-format check failed:\n${_CF_OUT}\n${_CF_ERR}")
else()
    message(STATUS "clang-format check passed")
endif()
