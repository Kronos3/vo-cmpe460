set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# Include the Options, and platform files. These are files that change the build
# setup. Users may need to add items to these files in order to ensure that all
# specific project builds work as expected. Since Options.cmake handles cache
# variables, the path handling is setup in between.
include(options)
include(settings)
#include(profile/profile)

# Sets up the build locations of the CMake system. This becomes the root of files
# being searched for in the cmake system.
set(FPRIME_BUILD_LOCATIONS "${FPRIME_FRAMEWORK_PATH}" ${FPRIME_LIBRARY_LOCATIONS} "${FPRIME_PROJECT_ROOT}")
list(REMOVE_DUPLICATES FPRIME_BUILD_LOCATIONS)
message(STATUS "Searching for F prime modules in: ${FPRIME_BUILD_LOCATIONS}")
message(STATUS "Autocoder constants file: ${FPRIME_AC_CONSTANTS_FILE}")
message(STATUS "Configuration header directory: ${FPRIME_CONFIG_DIR}")

include(platform/raspberry-pi)

include(utilities)
include(module)
include(required)

include(api)

# Setup the global include directories
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_BINARY_DIR}/F-Prime")

# Must always include the F prime core directory, as its headers are relative to
# that directory. Same with the project directory for separated projects.
include_directories("${FPRIME_PROJECT_ROOT}")
foreach (LIBRARY_DIR IN LISTS FPRIME_LIBRARY_LOCATIONS)
    # Including manifests from libraries
    file(GLOB MANIFESTS RELATIVE "${LIBRARY_DIR}" "${LIBRARY_DIR}/*.cmake")
    message(STATUS "Including library ${LIBRARY_DIR} with manifests ${MANIFESTS}")
    # Check to see if the cmake directory exists and add it
    if (IS_DIRECTORY "${LIBRARY_DIR}/cmake")
        list(APPEND CMAKE_MODULE_PATH "${LIBRARY_DIR}/cmake")
    endif()
    include_directories("${LIBRARY_DIR}")
endforeach()
include_directories("${FPRIME_FRAMEWORK_PATH}")
include_directories("${FPRIME_CONFIG_DIR}")
