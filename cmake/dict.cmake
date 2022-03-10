####
# dict.cmake:
#
# Dictionary target definition file. Used to define `dict` and `<MODULE>_dict` targets. Defined as
# a standard target pattern. This means that the following functions are defined:
#
# - `add_global_target`: adds a global target 'dict'
# - `add_module_target`: adds sub-targets for '<MODULE_NAME>_dict'
####

####
# Function `dict`:
#
# Generate a dictionary from any *AppAi.xml file that we see
####
function(dictgen MODULE_NAME AI_XML DEPS)
    string(REGEX REPLACE "Ai.xml" "Dictionary.xml" DICT_XML "${AI_XML}")
    string(REGEX REPLACE "Ai.xml" "ID.csv" ID_CSV_XML "${AI_XML}")
    string(REGEX REPLACE "Ai.xml" "Ai_IDTableLog.txt" ID_LOG_XML "${AI_XML}")
    string(REGEX REPLACE "Ai.xml" "Ac" AC_BASE "${AI_XML}")
    string(REPLACE ";" ":" FPRIME_BUILD_LOCATIONS_SEP "${FPRIME_BUILD_LOCATIONS}")
    get_filename_component(DICT_XML_NAME ${DICT_XML} NAME)
    get_filename_component(ID_CSV_XML_NAME ${ID_CSV_XML} NAME)
    get_filename_component(ID_LOG_XML_NAME ${ID_LOG_XML} NAME)
    set(DICT_ROOT "${FPRIME_INSTALL_DEST}/${TOOLCHAIN_NAME}/dict")
    set(DICTIONARY_OUTPUT_FILE "${DICT_ROOT}/${DICT_XML_NAME}")

    add_custom_command(
            OUTPUT "${DICT_ROOT}/${DICT_XML_NAME}"
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_COMMAND} -E env PYTHONPATH=${PYTHON_AUTOCODER_DIR}/src:${PYTHON_AUTOCODER_DIR}/utils BUILD_ROOT="${FPRIME_BUILD_LOCATIONS_SEP}"
            FPRIME_AC_CONSTANTS_FILE="${FPRIME_AC_CONSTANTS_FILE}"
            PYTHON_AUTOCODER_DIR=${PYTHON_AUTOCODER_DIR}
            ${FPRIME_FRAMEWORK_PATH}/Autocoders/Python/bin/codegen.py --build_root --xml_topology_dict ${AI_XML}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DICT_ROOT}"
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -E copy ${DICT_XML_NAME} ${ID_LOG_XML_NAME} ${ID_CSV_XML_NAME} ${DICT_ROOT}
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -E copy_directory commands "${DICT_ROOT}/commands"
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -E copy_directory channels "${DICT_ROOT}/channels"
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -E copy_directory events "${DICT_ROOT}/events"
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -E remove ${DICT_XML_NAME} ${ID_CSV_XML_NAME} ${ID_LOG_XML_NAME} ${AC_BASE}.cpp ${AC_BASE}.hpp
            # Workaround for older versions of cmake (~v3.10) that can only delete a single directory with "remove_directory" command.
            # When bumping cmake versions combine all deletions into a single "cmake -E rm -rf" command.
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -E remove_directory commands
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -E remove_directory channels
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_COMMAND} -E remove_directory events
            DEPENDS ${DEPS}
    )

    # Return file for output
    set(DICTIONARY_OUTPUT_FILE "${DICTIONARY_OUTPUT_FILE}" PARENT_SCOPE)
endfunction(dictgen)
