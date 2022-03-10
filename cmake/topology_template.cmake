function(TopologyTemplate template topology)
    execute_process(
            # Don't provide the output file to run in pretend mode
            # This mode will print out the file dependencies
            COMMAND ${PROJECT_SOURCE_DIR}/scripts/fprime-topology.py ${template} ${topology}
            OUTPUT_VARIABLE FILE_DEPENDENCIES
    )

    if(CMAKE_DEBUG_OUTPUT)
        message("Setting up Topology with dependencies ${FILE_DEPENDENCIES}")
    endif()

    add_custom_command(
            OUTPUT ${topology}
            COMMAND ${PROJECT_SOURCE_DIR}/scripts/fprime-topology.py ${template} ${topology}
            DEPEND ${template} ${FILE_DEPENDENCIES}
    )

    # This script will output the file dependencies as a CMake list (';' delimited)
    # Tell CMake that it needs to re-run configure() when these files are changed
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${template};${FILE_DEPENDENCIES}")

    if(ERR_RETURN)
        message(FATAL_ERROR "Failed to expand Topology template ${template}. ${ERR_RETURN}")
    endif()
endfunction()
