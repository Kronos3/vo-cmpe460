set(AUTOCODER_NAME autocoder/{{ AUTOCODER_NAME }})

# Import the autocoder implementation
include({{ AUTOCODER_FILENAME }})

####
# autocoder/autocoder.cmake:
#
# Autocoder setup and support file. This performs all of the general autocoder functions, running the specific functions
# defined within the individual autocoders. This gives the ability to run a set of autocoders to produce files.
#
# Note: autocoders need to be run by targets. See target/target.cmake.
####

####
# {{ AUTOCODER_NAME }}_run_ac:
#
# Main entrypoint to the autocoder generation
#
# SOURCES: source file input list
# ...: autocoder include or INFO_ONLY
####
function({{ AUTOCODER_NAME }}_run_ac SOURCES)
    set(AUTOCODER_NAME "{{ AUTOCODER_NAME }}")
    init_variables(MODULE_DEPENDENCIES GENERATED_FILES CONSUMED_SOURCES)
    __{{ AUTOCODER_NAME }}_run_ac("${SOURCES}" "${GENERATED_FILE_LIST}" "${INFO_ONLY}")

    list(APPEND AC_DEPENDENCIES ${MODULE_DEPENDENCIES})
    list(APPEND AC_GENERATED ${GENERATED_FILES})
    list(APPEND AC_SOURCES ${CONSUMED_SOURCES})
    set(AC_DEPENDENCIES "${AC_DEPENDENCIES}" PARENT_SCOPE)
    set(AC_GENERATED "${AC_GENERATED}" PARENT_SCOPE)
    set(AC_SOURCES "${AC_SOURCES}" PARENT_SCOPE)
endfunction()


####
# __{{ AUTOCODER_NAME }}_memoize:
#
# This take two "long" processing steps of the autocoder that run during the configuration step of CMake and notes the
# output such that the results are cached and repeated unless the input file has changed since the last pass through
# this function. This is done for efficiency when the generate dependencies or generate files step takes a large
# execution costs.
#
# Note: cached variables are the following: GENERATED_FILES, MODULE_DEPENDENCIES, FILE_DEPENDENCIES
#
# SOURCES: source file that is being parsed and must have changed for a recalculation
####
function(__{{ AUTOCODER_NAME }}_memoize SOURCES)
    set(SOURCES_HASH "multiple")
    if({{ AUTOCODER_NAME }}_HANDLES_INDIVIDUAL_SOURCES)
        string(MD5 SOURCES_HASH "${SOURCES}")
    endif()
    set(MEMO_FILE "${CMAKE_CURRENT_BINARY_DIR}/${AUTOCODER_NAME}.${SOURCES_HASH}.dep")

    {{ AUTOCODER_NAME }}_regenerate_memo(FORCE_REGENERATE "${MEMO_FILE}" "${SOURCES}")
    if(FORCE_REGENERATE)
        {{ AUTOCODER_NAME }}_get_generated_files("${SOURCES}")
        {{ AUTOCODER_NAME }}_get_dependencies("${SOURCES}")
        resolve_dependencies(MODULE_DEPENDENCIES ${MODULE_DEPENDENCIES})
        file(WRITE "${MEMO_FILE}" "${GENERATED_FILES}\n${MODULE_DEPENDENCIES}\n${FILE_DEPENDENCIES}\n${EXTRAS}\n${LAST_DEP_COMMAND}\n")
        # Otherwise read from file
    else()
        if(CMAKE_DEBUG_OUTPUT)
            message(STATUS "[Autocode/${AUTOCODER_NAME}] Using memo ${MEMO_FILE}'")
        endif()
        file(READ "${MEMO_FILE}" CONTENTS)
        read_from_lines("${CONTENTS}" GENERATED_FILES MODULE_DEPENDENCIES FILE_DEPENDENCIES EXTRAS)
    endif()
    set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
    set(MODULE_DEPENDENCIES "${MODULE_DEPENDENCIES}" PARENT_SCOPE)
    set(FILE_DEPENDENCIES "${FILE_DEPENDENCIES}" PARENT_SCOPE)
    set(EXTRAS "${EXTRAS}" PARENT_SCOPE)
endfunction()

####
# run_ac:
#
# Run the autocoder across the set of source files, SOURCES, and the previously generated sources, GENERATED_SOURCES.
# This will filter the SOURCES and GENERATED_SOURCES down to the handled set. Then for single-input autocoders, it runs
# the autocoder one input at a time, otherwise it runs the autocoder once on all inputs.
#
# SOURCES: sources input to run on the autocoder
# GENERATED_SOURCES: sources created by other autocoders
# INFO_ONLY: TRUE if only information is needed, FALSE to run otherwise
####
function(__{{ AUTOCODER_NAME }}_run_ac SOURCES GENERATED_SOURCES INFO_ONLY)
    normalize_paths(AC_INPUT_SOURCES "${SOURCES}" "${GENERATED_SOURCES}")

    # Filter only the files that can be processed by this autocoder
    set(AC_INPUT_SOURCES_T)
    foreach(SOURCE IN LISTS SOURCES)
        {{ AUTOCODER_NAME }}_is_supported("${SOURCE}")
        if(IS_SUPPORTED)
            list(APPEND AC_INPUT_SOURCES_T "${SOURCE}")
        endif()
    endforeach()

    set(AC_INPUT_SOURCES "${AC_INPUT_SOURCES_T}")
    if(NOT AC_INPUT_SOURCES)
        if(CMAKE_DEBUG_OUTPUT)
            message(STATUS "[Autocode/${AUTOCODER_NAME}] No sources detected")
        endif()
        return()
    endif()

    # Start by displaying inputs to autocoders
    if(CMAKE_DEBUG_OUTPUT AND NOT INFO_ONLY)
        message(STATUS "[Autocode/${AUTOCODER_NAME}] Autocoding Input Sources:")
        foreach(SOURCE IN LISTS AC_INPUT_SOURCES)
            message(STATUS "[Autocode/${AUTOCODER_NAME}]   ${SOURCE}")
        endforeach()
    endif()

    set(CONSUMED_SOURCES)
    # Handles individual/multiple source handling
    if(HANDLES_INDIVIDUAL_SOURCES)
        init_variables(MODULE_DEPENDENCIES_LIST GENERATED_FILES_LIST)
        foreach(SOURCE IN LISTS AC_INPUT_SOURCES)
            __{{ AUTOCODER_NAME }}_ac_process_sources("${SOURCE}" "${INFO_ONLY}")
            list(APPEND MODULE_DEPENDENCIES_LIST ${MODULE_DEPENDENCIES})
            list(APPEND GENERATED_FILES_LIST ${GENERATED_FILES})
            # Check if this would have generated something, if not don't mark the file as used
            if(GENERATED_FILES)
                list(APPEND CONSUMED_SOURCES "${SOURCE}")
            endif()
        endforeach()
        set(MODULE_DEPENDENCIES "${MODULE_DEPENDENCIES_LIST}")
        set(GENERATED_FILES "${GENERATED_FILES_LIST}")
    else()
        __{{ AUTOCODER_NAME }}_ac_process_sources("${AC_INPUT_SOURCES}" "${INFO_ONLY}")
        if(GENERATED_FILES)
            set(CONSUMED_SOURCES "${AC_INPUT_SOURCES}")
        endif()
    endif()
    # When actually generating items, explain what is done and why
    if(CMAKE_DEBUG_OUTPUT AND NOT INFO_ONLY)
        message(STATUS "[Autocode/${AUTOCODER_NAME}] Generated Files:")
        foreach(GENERATED_FILE IN LISTS GENERATED_FILES)
            message(STATUS "[Autocode/${AUTOCODER_NAME}]   ${GENERATED_FILE}")
        endforeach()
        # Output file dependency status block
        if(FILE_DEPENDENCIES)
            message(STATUS "[Autocode/${AUTOCODER_NAME}] File Dependencies:")
            foreach(FILE_DEPENDENCY IN LISTS FILE_DEPENDENCIES)
                message(STATUS "[Autocode/${AUTOCODER_NAME}]   ${FILE_DEPENDENCY}")
            endforeach()
        endif()
        # Output module dependency status block
        if(MODULE_DEPENDENCIES)
            message(STATUS "[Autocode/${AUTOCODER_NAME}] Module Dependencies:")
            foreach(MODULE_DEPENDENCY IN LISTS MODULE_DEPENDENCIES)
                message(STATUS "[Autocode/${AUTOCODER_NAME}]   ${MODULE_DEPENDENCY}")
            endforeach()
        endif()
    endif()
    # Configure depends on this source file if it causes a change to module dependencies
    if(MODULE_DEPENDENCIES)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CONSUMED_SOURCES})
    endif()

    # Return variables
    set(MODULE_DEPENDENCIES "${MODULE_DEPENDENCIES}" PARENT_SCOPE)
    set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)
    set(CONSUMED_SOURCES "${CONSUMED_SOURCES}" PARENT_SCOPE)
endfunction(__{{ AUTOCODER_NAME }}_run_ac)

####
# __{{ AUTOCODER_NAME }}_ac_process_sources:
#
# Process sources found in SOURCES list.  If INFO_ONLY is set, then the autocoder is not setup, but rather only the
# dependency and file information is generated. Otherwise, the autocoder is setup to run on sources. Helper function.
#
# SOURCES: source file list. Note: if the autocoder sets HANDLES_INDIVIDUAL_SOURCES this will be singular
# INFO_ONLY: TRUE if only information is to be generated FALSE (normal) to setup autocoding rules
####
function(__{{ AUTOCODER_NAME }}_ac_process_sources SOURCES INFO_ONLY)
    # Run the autocode setup process now with memoization
    __{{ AUTOCODER_NAME }}_memoize("${SOURCES}")
    set(MODULE_DEPENDENCIES "${MODULE_DEPENDENCIES}" PARENT_SCOPE)
    set(GENERATED_FILES "${GENERATED_FILES}" PARENT_SCOPE)

    # Run the generation setup when not requesting "info only"
    if(NOT INFO_ONLY)
        {{ AUTOCODER_NAME }}_setup_autocode("${SOURCES}" "${GENERATED_FILES}" "${MODULE_DEPENDENCIES}" "${FILE_DEPENDENCIES}" "${EXTRAS}")
    endif()
endfunction()

