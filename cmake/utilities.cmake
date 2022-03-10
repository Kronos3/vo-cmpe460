####
# init_variables:
#
# Initialize all variables passed in to empty variables in the calling scope.
####
function(init_variables)
    foreach (VARIABLE IN LISTS ARGN)
        set(${VARIABLE} PARENT_SCOPE)
    endforeach()
endfunction(init_variables)


####
# normalize_paths:
#
# Take in any number of lists of paths and normalize the paths returning a single list.
# OUTPUT_NAME: name of variable to set in parent scope
####
function(normalize_paths OUTPUT_NAME)
    set(OUTPUT_LIST)
    # Loop over the list and check
    foreach (PATH_LIST IN LISTS ARGN)
        foreach(PATH IN LISTS PATH_LIST)
            get_filename_component(PATH "${PATH}" REALPATH)
            list(APPEND OUTPUT_LIST "${PATH}")
        endforeach()
    endforeach()
    set(${OUTPUT_NAME} "${OUTPUT_LIST}" PARENT_SCOPE)
endfunction(normalize_paths)

####
# resolve_dependencies:
#
# Sets OUTPUT_VAR in parent scope to be the set of dependencies in canonical form: relative path from root replacing
# directory separators with "_".  E.g. fprime/Fw/Time becomes Fw_Time.
#
# OUTPUT_VAR: variable to fill in parent scope
# ARGN: list of dependencies to resolve
####
function(resolve_dependencies OUTPUT_VAR)
    # Resolve all dependencies
    set(RESOLVED)
    foreach(DEPENDENCY IN LISTS ARGN)
        get_module_name(${DEPENDENCY})
        if (NOT MODULE_NAME IN_LIST RESOLVED)
            list(APPEND RESOLVED "${MODULE_NAME}")
        endif()
    endforeach()
    set(${OUTPUT_VAR} "${RESOLVED}" PARENT_SCOPE)
endfunction(resolve_dependencies)


####
# build_relative_path:
#
# Calculate the path to an item relative to known build paths.  Search is performed in the following order erring if the
# item is found in multiple paths.
#
# INPUT_PATH: input path to search
# OUTPUT_VAR: output variable to fill
####
function(build_relative_path INPUT_PATH OUTPUT_VAR)
    # Implementation assertion
    if (NOT DEFINED FPRIME_BUILD_LOCATIONS)
        message(FATAL_ERROR "FPRIME_BUILD_LOCATIONS not set before build_relative_path was called")
    endif()
    normalize_paths(FPRIME_LOCS_NORM ${FPRIME_BUILD_LOCATIONS})
    normalize_paths(INPUT_PATH ${INPUT_PATH})
    foreach(PARENT IN LISTS FPRIME_LOCS_NORM)
        string(REGEX REPLACE "${PARENT}/(.*)$" "\\1" LOC_TEMP "${INPUT_PATH}")
        if (NOT LOC_TEMP STREQUAL INPUT_PATH AND NOT LOC_TEMP MATCHES "${LOC}$")
            message(FATAL_ERROR "Found ${INPUT_PATH} at multiple locations: ${LOC} and ${LOC_TEMP}")
        elseif(NOT LOC_TEMP STREQUAL INPUT_PATH AND NOT DEFINED LOC)
            set(LOC "${LOC_TEMP}")
        endif()
    endforeach()
    if (LOC STREQUAL "")
        message(FATAL_ERROR "Failed to find location for: ${INPUT_PATH}")
    endif()
    set(${OUTPUT_VAR} ${LOC} PARENT_SCOPE)
endfunction(build_relative_path)

####
# on_any_changed:
#
# Sets VARIABLE to true if any file has been noted as changed from the "on_changed" function.  Will create cache files
# in the binary directory.  Please see: on_changed
#
# INPUT_FILES: files to check for changes
# ARGN: passed into execute_process via on_changed call
####
function (on_any_changed INPUT_FILES VARIABLE)
    foreach(INPUT_FILE IN LISTS INPUT_FILES)
        on_changed("${INPUT_FILE}" TEMP_ON_CHANGED ${ARGN})
        if (TEMP_ON_CHANGED)
            set(${VARIABLE} TRUE PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${VARIABLE} FALSE PARENT_SCOPE)
endfunction()

####
# on_changed:
#
# Sets VARIABLE to true if and only if the given file has changed since the last time this function was invoked. It will
# create "${INPUT_FILE}.prev" in the binary directory as a cache from the previous invocation. The result is always TRUE
# unless a successful no-difference is calculated.
#
# INPUT_FILE: file to check if it has changed
# ARGN: passed into execute_process
####
function (on_changed INPUT_FILE VARIABLE)
    get_filename_component(INPUT_BASENAME "${INPUT_FILE}" NAME)
    set(PREVIOUS_FILE "${CMAKE_CURRENT_BINARY_DIR}/${INPUT_BASENAME}.prev")

    execute_process(COMMAND "${CMAKE_COMMAND}" -E compare_files "${INPUT_FILE}" "${PREVIOUS_FILE}"
                    RESULT_VARIABLE difference OUTPUT_QUIET ERROR_QUIET)
    # Files are the same, leave this function
    if (difference EQUAL 0)
        set(${VARIABLE} FALSE PARENT_SCOPE)
        return()
    endif()
    set(${VARIABLE} TRUE PARENT_SCOPE)
    # Update the file with the latest
    if (EXISTS "${INPUT_FILE}")
        execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${INPUT_FILE}" "${PREVIOUS_FILE}" OUTPUT_QUIET)
    endif()
endfunction()

####
# read_from_lines:
#
# Reads a set of variables from a newline delimited test base. This will read each variable as a separate line. It is
# based on the number of arguments passed in.
####
function (read_from_lines CONTENT)
    # Loop through each arg
    foreach(NAME IN LISTS ARGN)
        string(REGEX MATCH   "^([^\r\n]+)" VALUE "${CONTENT}")
        string(REGEX REPLACE "^([^\r\n]*)\r?\n(.*)" "\\2" CONTENT "${CONTENT}")
        set(${NAME} "${VALUE}" PARENT_SCOPE)
    endforeach()
endfunction()

####
# Function `get_module_name`:
#
# Takes a path, or something path-like and returns the module's name. This breaks down as the
# following:
#
#  1. If passed a path, the module name is the '_'ed variant of the relative path from BUILD_ROOT
#  2. If passes something which does not exist on the file system, it is just '_'ed
#
# i.e. ${BUILD_ROOT}/Svc/ActiveLogger becomes Svc_ActiveLogger
#      Svc/ActiveLogger also becomes Svc_ActiveLogger
#
# - **DIRECTORY_PATH:** path to infer MODULE_NAME from
# - **Return: MODULE_NAME** (set in parent scope)
####
function(get_module_name DIRECTORY_PATH)
    # If DIRECTORY_PATH exists, then find its offset from BUILD_ROOT to calculate the module
    # name. If it does not exist, then it is assumed to be an offset already and is carried
    # forward in the calculation.
    if (EXISTS ${DIRECTORY_PATH} AND IS_ABSOLUTE ${DIRECTORY_PATH})
        # Module names a based on the current directory, not a file
        if (NOT IS_DIRECTORY ${DIRECTORY_PATH})
            get_filename_component(DIRECTORY_PATH "${DIRECTORY_PATH}" DIRECTORY)
        endif()
        File(RELATIVE_PATH TEMP_MODULE_NAME ${FPRIME_CLOSEST_BUILD_ROOT} ${DIRECTORY_PATH})
    else()
        set(TEMP_MODULE_NAME ${DIRECTORY_PATH})
    endif()
    # Replace slash with underscore to have valid name
    string(REPLACE "/" "_" TEMP_MODULE_NAME ${TEMP_MODULE_NAME})
    set(MODULE_NAME ${TEMP_MODULE_NAME} PARENT_SCOPE)
endfunction(get_module_name)


####
# Function `set_hash_flag`:
#
# Adds a -DASSERT_FILE_ID=(First 8 digits of MD5) to each source file, and records the output in
# hashes.txt. This allows for asserts on file ID not string.
####
function(set_hash_flag SRC)
    get_filename_component(FPRIME_CLOSEST_BUILD_ROOT_ABS "${FPRIME_CLOSEST_BUILD_ROOT}" ABSOLUTE)
    string(REPLACE "${FPRIME_CLOSEST_BUILD_ROOT_ABS}/" "" SHORT_SRC "${SRC}")
    string(MD5 HASH_VAL "${SHORT_SRC}")
    string(SUBSTRING "${HASH_VAL}" 0 8 HASH_32)
    file(APPEND "${CMAKE_BINARY_DIR}/hashes.txt" "${SHORT_SRC}: 0x${HASH_32}\n")
    SET_SOURCE_FILES_PROPERTIES(${SRC} PROPERTIES COMPILE_FLAGS -DASSERT_FILE_ID="0x${HASH_32}")
endfunction(set_hash_flag)


####
# Function `print_property`:
#
# Prints a given property for the module.
# - **TARGET**: target to print properties
# - **PROPERTY**: name of property to print
####
function (print_property TARGET PROPERTY)
    get_target_property(OUT "${TARGET}" "${PROPERTY}")
    if (NOT OUT MATCHES ".*-NOTFOUND")
        message(STATUS "[F´ Module] ${TARGET} ${PROPERTY}:")
        foreach (PROPERTY IN LISTS OUT)
            message(STATUS "[F´ Module]    ${PROPERTY}")
        endforeach()
    endif()
endfunction(print_property)

####
# Function `introspect`:
#
# Prints the dependency list of the module supplied as well as the include directories.
#
# - **MODULE_NAME**: module name to print dependencies
####
function(introspect MODULE_NAME)
    print_property("${MODULE_NAME}" SOURCES)
    print_property("${MODULE_NAME}" INCLUDE_DIRECTORIES)
    print_property("${MODULE_NAME}" LINK_LIBRARIES)
endfunction(introspect)

####
# Function `fprime_ai_info`:
#
# A function used to detect all the needed information for an Ai.xml file. This looks for the following items:
#  1. Type of object defined inside: Component, Port, Enum, Serializable, TopologyApp
#  2. All fprime module dependencies that may be auto-detected
#  3. All file dependencies
#
# - **XML_PATH:** full path to the XML used for sources.
# - **MODULE_NAME:** name of the module soliciting new dependencies
####
function(fprime_ai_info XML_PATH MODULE_NAME)
    # Run the parser and capture the output. If an error occurs, that fatals CMake as we cannot continue
    set(MODULE_NAME_NO_SUFFIX "${MODULE_NAME}")
    execute_process(
            COMMAND "${FPRIME_FRAMEWORK_PATH}/cmake/support/parser/ai_parser.py" "${XML_PATH}" "${MODULE_NAME_NO_SUFFIX}" "${FPRIME_CLOSEST_BUILD_ROOT}"
            RESULT_VARIABLE ERR_RETURN
            OUTPUT_VARIABLE AI_OUTPUT
    )
    if(ERR_RETURN)
        message(FATAL_ERROR "Failed to parse ${XML_PATH}. ${ERR_RETURN}")
    endif()
    # Next parse the output matching one line at a time, then consuming it and matching the next
    string(REGEX MATCH   "([^\r\n]+)" XML_TYPE "${AI_OUTPUT}")
    string(REGEX REPLACE "([^\r\n]+)\r?\n(.*)" "\\2" AI_OUTPUT "${AI_OUTPUT}")
    string(REGEX MATCH   "^([^\r\n]+)" MODULE_DEPENDENCIES "${AI_OUTPUT}")
    string(REGEX REPLACE "([^\r\n]+)\r?\n(.*)" "\\2" AI_OUTPUT "${AI_OUTPUT}")
    string(REGEX MATCH   "^([^\r\n]+)" FILE_DEPENDENCIES "${AI_OUTPUT}")

    # Next compute the needed variants of the items needed. This
    string(TOLOWER ${XML_TYPE} XML_LOWER_TYPE)
    get_filename_component(XML_NAME "${INPUT_FILE}" NAME)
    string(REGEX REPLACE "(${XML_TYPE})?Ai.xml" "" AC_OBJ_NAME "${XML_NAME}")

    # Finally, set all variables into parent scope
    set(XML_TYPE "${XML_TYPE}" PARENT_SCOPE)
    set(XML_LOWER_TYPE "${XML_LOWER_TYPE}" PARENT_SCOPE)
    set(AC_OBJ_NAME "${AC_OBJ_NAME}" PARENT_SCOPE)
    set(MODULE_DEPENDENCIES "${MODULE_DEPENDENCIES}" PARENT_SCOPE)
    set(FILE_DEPENDENCIES "${FILE_DEPENDENCIES}" PARENT_SCOPE)
endfunction(fprime_ai_info)

####
# Function `get_target_name`:
#
# Gets the target name from the path to the target file. Two variants of this name will be
# generated and placed in parent scope: TARGET_NAME, and TARGET_MOD_NAME.
#
# - **MODULE_NAME:** module name for TARGET_MOD_NAME variant
# - **Return: TARGET_NAME** (set in parent scope), global target name i.e. `dict`.
# - **Return: TARGET_MOD_NAME** (set in parent scope), module target name. i.e. `Fw_Cfg_dict`
#
# **Note:** TARGET_MOD_NAME not set if optional argument `MODULE_NAME` not supplied
####
function(get_target_name)
    get_filename_component(BASE_VAR ${ARGV0} NAME_WE)
    set(TARGET_NAME ${BASE_VAR} PARENT_SCOPE)
    if (${ARGC} GREATER 1)
        set(TARGET_MOD_NAME "${ARGV1}_${BASE_VAR}" PARENT_SCOPE)
    endif()
endfunction(get_target_name)
