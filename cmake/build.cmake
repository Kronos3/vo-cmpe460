####
# setup_build_module:
#
# Helper function to setup the module. This was the historical core of the CMake system, now embedded as part of this
# build target. It adds a the target (library, executable), sets up compiler source files, flags generated sources,
# sets up module and linker dependencies adds the file to the hashes.txt file, sets up include directories, etc.
#
# - MODULE: module name being setup
# - SOURCES: hand-specified source files
# - GENERATED: generated sources
# - EXCLUDED_SOURCES: sources already "consumed", that is, processed by an autocoder
# - DEPENDENCIES: dependencies of this module. Also link flags and libraries.
####
function(setup_build_module MODULE SOURCES GENERATED EXCLUDED_SOURCES DEPENDENCIES)
    # Compilable sources
    set(COMPILE_SOURCES)
    foreach(SOURCE IN LISTS SOURCES GENERATED)
        if (NOT SOURCE IN_LIST EXCLUDED_SOURCES)
            list(APPEND COMPILE_SOURCES "${SOURCE}")
        endif()
    endforeach()
    # Setup the actual target
    if (FPRIME_OBJECT_TYPE STREQUAL "Library")
        # Add the library name
        add_library(${MODULE} ${COMPILE_SOURCES})
    else()
        add_executable(${MODULE} ${COMPILE_SOURCES})
    endif()

    # Set those files as generated to prevent build errors
    foreach(SOURCE IN LISTS GENERATED)
        set_source_files_properties(${SOURCE} PROPERTIES GENERATED TRUE)
    endforeach()
    # Setup the hash file for our sources
    foreach(SRC_FILE ${COMPILE_SOURCES})
        set_hash_flag("${SRC_FILE}")
    endforeach()

    # Includes the source, so that the Ac files can include source headers
    target_include_directories("${MODULE}" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    # For every detected dependency, add them to the supplied module. This enforces build order.
    # Also set the link dependencies on this module. CMake rolls-up link dependencies, and thus
    # this prevents the need for manually specifying link orders.
    set(TARGET_DEPENDENCIES)
    foreach(DEPENDENCY ${DEPENDENCIES})
        if (NOT (DEPENDENCY MATCHES "^-l.*" OR DEPENDENCY MATCHES "version") )
            add_dependencies(${MODULE} "${DEPENDENCY}")
            list(APPEND TARGET_DEPENDENCIES "${DEPENDENCY}")
        endif()
        target_link_libraries(${MODULE} PUBLIC "${DEPENDENCY}")
    endforeach()
    set_property(TARGET "${MODULE}" PROPERTY FPRIME_TARGET_DEPENDENCIES ${TARGET_DEPENDENCIES})
endfunction()
