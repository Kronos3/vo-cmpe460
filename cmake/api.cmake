function(fprime_enter_root dir)
    set(FPRIME_CLOSEST_BUILD_ROOT "${dir}" PARENT_SCOPE)
endfunction()

function(register_fprime_module)
    # SOURCE_FILES is supplied as the first positional -OR- as the list 'SOURCE_FILES'
    if (${ARGC} GREATER 0)
        set(SC_IFS "${ARGV0}")
    elseif(DEFINED SOURCE_FILES)
        set(SC_IFS "${SOURCE_FILES}")
    else()
        message(FATAL_ERROR "'SOURCE_FILES' not defined in '${CMAKE_CURRENT_LIST_FILE}'.")
    endif()
    # MOD_DEPS is supplied as an optional second positional -OR- or as  the list 'MOD_DEPS'
    if (${ARGC} GREATER 1)
        set(MD_IFS "${ARGV1}")
    elseif(DEFINED MOD_DEPS)
        set(MD_IFS "${MOD_DEPS}")
    elseif(${CMAKE_DEBUG_OUTPUT})
        message(STATUS "No extra 'MOD_DEPS' found in '${CMAKE_CURRENT_LIST_FILE}'.")
    endif()
    if (${ARGC} GREATER 2)
        set(MODULE_NAME "${ARGV2}")
    else()
        # Sets MODULE_NAME to unique name based on path, and then adds the library of
        get_module_name(${CMAKE_CURRENT_LIST_DIR})
    endif()
    # Explicit call to module register
    generate_library("${MODULE_NAME}" "${SOURCE_FILES}" "${MOD_DEPS}")
endfunction(register_fprime_module)

function(register_fprime_executable)
    if (${ARGC} GREATER 0)
        set(EX_NAME "${ARGV0}")
    elseif(DEFINED EXECUTABLE_NAME)
        set(EX_NAME ${EXECUTABLE_NAME})
    elseif(DEFINED PROJECT_NAME)
        set(EX_NAME "${PROJECT_NAME}")
    else()
        message(FATAL_ERROR "'EXECUTABLE_NAME' not defined in '${CMAKE_CURRENT_LIST_FILE}'.")
    endif()
    # SOURCE_FILES is supplied as the first positional -OR- as the list 'SOURCE_FILES'
    if (${ARGC} GREATER 1)
        set(SC_IFS "${ARGV1}")
    elseif(DEFINED SOURCE_FILES)
        set(SC_IFS "${SOURCE_FILES}")
    else()
        message(FATAL_ERROR "'SOURCE_FILES' not defined in '${CMAKE_CURRENT_LIST_FILE}'.")
    endif()
    # MOD_DEPS is supplied as an optional second positional -OR- or as  the list 'MOD_DEPS'
    if (${ARGC} GREATER 2)
        set(MD_IFS "${ARGV2}")
    elseif(DEFINED MOD_DEPS)
        set(MD_IFS "${MOD_DEPS}")
    elseif(${CMAKE_DEBUG_OUTPUT})
        message(STATUS "No extra 'MOD_DEPS' found in '${CMAKE_CURRENT_LIST_FILE}'.")
    endif()
    # Register executable and module with name '<exe name>_exe', then create an empty target with
    # name '<exe name>' that depends on the executable. This enables additional post-processing
    # targets that depend on the built executable.
    generate_executable("${EXECUTABLE_NAME}_exe" "${SOURCE_FILES}" "${MOD_DEPS}")
    set_target_properties("${EXECUTABLE_NAME}_exe" PROPERTIES OUTPUT_NAME "${EXECUTABLE_NAME}")
    add_custom_target(${EXECUTABLE_NAME} ALL)
    add_dependencies("${EXECUTABLE_NAME}" "${EXECUTABLE_NAME}_exe")
endfunction(register_fprime_executable)

function(add_fprime_subdirectory FP_SOURCE_DIR)
    # Check if the binary and source directory are in agreement. If they agree, then normally add
    # the directory, as no adjustments need be made.
    get_filename_component(CBD_NAME "${CMAKE_CURRENT_BINARY_DIR}" NAME)
    get_filename_component(CSD_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    if ("${CBD_NAME}" STREQUAL "${CSD_NAME}")
        add_subdirectory(${ARGV}) # List of all args, not just extras
        return()
    endif()
    if (${ARGC} GREATER 2)
        message(FATAL_ERROR "Cannot use 'add_fprime_subdirectory' with [binary_dir] argument.")
    endif()
#    get_nearest_build_root("${FP_SOURCE_DIR}")
#    file(RELATIVE_PATH NEW_BIN_DIR "${FPRIME_CLOSEST_BUILD_ROOT}" "${FP_SOURCE_DIR}")
    # Add component subdirectories using normal add_subdirectory with overridden binary_dir
    add_subdirectory("${FP_SOURCE_DIR}" "${NEW_BIN_DIR}" ${ARGN})
endfunction(add_fprime_subdirectory)

function(register_fprime_ut)
    # SSim UT are not generated by CMake
    # This is a no-op
endfunction()

