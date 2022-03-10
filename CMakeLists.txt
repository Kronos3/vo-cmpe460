cmake_minimum_required(VERSION 3.15)

# Initialize the compiler toolchain
include(toolchain.cmake)

project(VO-CMPE460)
add_compile_options(-pipe)

include(ExternalProject)
ExternalProject_Add(
        usbboot
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/usbboot
        BINARY_DIR ${CMAKE_BINARY_DIR}/usbboot
        CONFIGURE_COMMAND "${CMAKE_COMMAND}" -E make_directory ${CMAKE_BINARY_DIR}/usbboot
        BUILD_COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_SOURCE_DIR}/usbboot
        INSTALL_COMMAND ""
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(AARCH 32)
set(RASPPI 1)
set(ARM_ALLOW_MULTI_CORE OFF)
set(FLOAT_ABI "hard")
include(circle/circle.cmake)

# Override default FPrime configuration
set(FPRIME_CONFIG_DIR "${PROJECT_SOURCE_DIR}/src/cfg")
set(FPRIME_AC_CONSTANTS_FILE "${FPRIME_CONFIG_DIR}/AiConstants.txt")

##
# Required for FSW FPrime CMake configuration
##
set(FPRIME_FRAMEWORK_PATH ${PROJECT_SOURCE_DIR}/fprime)

# Set the locations to search for FPrime components
set(FPRIME_PROJECT_ROOT ${PROJECT_SOURCE_DIR})
set(FPRIME_INSTALL_DEST ${PROJECT_SOURCE_DIR})
set(FPRIME_LIBRARY_LOCATIONS
        ${FPRIME_FRAMEWORK_PATH}
        ${PROJECT_SOURCE_DIR}
        )

##
# FPrime instantiation
##
#set(CMAKE_DEBUG_OUTPUT TRUE)  # debug the CMake configuration
include(cmake/fprime.cmake)

# Set the framework libraries provided by FPrime
fprime_enter_root(${PROJECT_SOURCE_DIR}/fprime)
include(cmake/fprime_code.cmake)

fprime_enter_root(${PROJECT_SOURCE_DIR})
add_subdirectory(circle)   # driver implementations
add_subdirectory(src)   # FSW

cmake_policy(SET CMP0079 NEW)

add_subdirectory(circle/sample/09-softserial)
add_subdirectory(circle/sample/03-screentext)
