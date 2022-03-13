cmake_minimum_required(VERSION 3.15)

project(VO-CMPE460)
enable_language(C CXX ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(AARCH 32)
set(RASPPI 1)
set(ARM_ALLOW_MULTI_CORE OFF)
set(FLOAT_ABI "hard")

# Initialize the compiler toolchain
include(toolchain.cmake)
include(circle/circle.cmake)

# Add circle drivers
add_subdirectory(circle/lib)
add_subdirectory(circle/addon)

target_include_directories(circle PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/circle/include
        ${CMAKE_CURRENT_LIST_DIR}/circle/addon)

add_compile_options(-pipe)

# Override default FPrime configuration
set(FPRIME_CONFIG_DIR "${PROJECT_SOURCE_DIR}/Cfg")
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

fprime_enter_root(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(Drv)   # Raspberry Pi drivers
add_subdirectory(Rpi)   # FSW

cmake_policy(SET CMP0079 NEW)

add_subdirectory(circle/sample/09-softserial)
add_subdirectory(circle/sample/03-screentext)
