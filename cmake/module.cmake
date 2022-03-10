include(autocoder/autocoder)
include(build)
include(dict)

function(generate_module MODULE_NAME SOURCES DEPENDENCIES)
    message(STATUS "Adding ${FPRIME_OBJECT_TYPE}: ${MODULE_NAME}")

    # Initialize the global autocoder listing
    init_variables(AC_DEPENDENCIES AC_GENERATED AC_SOURCES)

    # Run FPP autocoder where applicable
    fpp_run_ac("${SOURCES}")

    # Run XML autocoder where applicable
    ai_xml_run_ac("${SOURCES};${AC_GENERATED}")  # chain the dependencies, fpp -> ai_xml -> c++

    # Resolve all module dependencies
    resolve_dependencies(RESOLVED ${DEPENDENCIES} ${AC_DEPENDENCIES} )
    if(CMAKE_DEBUG_OUTPUT)
        message(STATUS "[Module] Resolved dependencies:")
        foreach(MODULE_DEPENDENCY IN LISTS RESOLVED)
            message(STATUS "[Module]   ${MODULE_DEPENDENCY}")
        endforeach()
    endif()
    setup_build_module("${MODULE_NAME}" "${SOURCES}" "${AC_GENERATED}" "${AC_SOURCES}" "${RESOLVED}")

    # Generate dictionaries where applicable
    # Try to generate dictionaries for every AC input file
    foreach (AC_IN ${SOURCES})
        # Only generate dictionaries on serializables or topologies
        if (AC_IN MATCHES ".*Topology.*\.xml$")
            fprime_ai_info("${AC_IN}" "${MODULE_NAME}")
            dictgen("${MODULE_NAME}" "${AC_IN}" "${AC_IN};${MODULE_DEPENDENCIES};${RESOLVED};${FILE_DEPENDENCIES}")
            add_custom_target("${MODULE_NAME}_dict" DEPENDS "${AC_IN}" "${DICTIONARY_OUTPUT_FILE}")
            add_dependencies("${MODULE_NAME}" "${MODULE_NAME}_dict")
        endif()
    endforeach()
endfunction()

####
# Function `generate_executable:`
#
# Top-level executable generation. Core allows for generation of UT specifics without affecting API.
#
# - **EXECUTABLE_NAME:** name of executable to be generated.
# - **SOURCE_FILES:** source files for this executable, split into AC and normal sources
# - **DEPENDENCIES:** specified module-level dependencies
####
function(generate_executable EXECUTABLE_NAME SOURCE_FILES DEPENDENCIES)
    set(FPRIME_OBJECT_TYPE "Executable")
    generate_module("${EXECUTABLE_NAME}" "${SOURCE_FILES}" "${DEPENDENCIES}")
endfunction(generate_executable)

####
# Function `generate_library`:
#
# Generates a library as part of F prime. This runs the AC and all the other items for the build.
# It takes SOURCE_FILES_INPUT and DEPS_INPUT, splits them up into ac sources, sources, mod deps,
# and library deps.
# - *MODULE_NAME:* module name of library to build
# - *SOURCE_FILES:* source files that will be split into AC and normal sources.
# - *DEPENDENCIES:* dependencies bound for link and cmake dependencies
#
####
function(generate_library MODULE_NAME SOURCE_FILES DEPENDENCIES)
    set(FPRIME_OBJECT_TYPE "Library")
    generate_module("${MODULE_NAME}" "${SOURCE_FILES}" "${DEPENDENCIES}")
endfunction(generate_library)
