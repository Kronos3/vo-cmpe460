cmake_minimum_required(VERSION 3.15)

project(VO-CMPE460)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(AARCH 64)
set(RASPPI 3)
set(ARM_ALLOW_MULTI_CORE OFF)
set(FLOAT_ABI "hard")

# 'bare_metal' will build Circle + FPrime layer
# 'rt_linux' will run on Preempt RT Linux and build /sbin/init
set(RPI_PLATFORM rt_linux)

enable_language(C CXX ASM)

# Initialize the compiler toolchain
#include(cmake/platform/${RPI_PLATFORM}-toolchain-clang.cmake)
include(cmake/platform/${RPI_PLATFORM}-toolchain-gcc.cmake)

#add_compile_options(-fsanitize=addressss -fno-omit-frame-pointer -O1)
#add_link_options(-fsanitize=address)

#find_package(OpenCV REQUIRED)
#include_directories(${OpenCV_INCLUDE_DIRS})

set(STAGING_DIR ${CMAKE_CURRENT_LIST_DIR}/staging)
link_directories(${STAGING_DIR}/lib)
include_directories(
        ${STAGING_DIR}/include/
        ${STAGING_DIR}/include/libcamera/
        ${STAGING_DIR}/include/libdrm/
        ${STAGING_DIR}/include/opencv4
)

#add_compile_options(-isystem /usr/include)

if (RPI_PLATFORM EQUAL "bare_metal")
    add_subdirectory(circle)
endif()

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
#        ${PROJECT_SOURCE_DIR}/Kernel/${RPI_PLATFORM}
        )

#include_directories(
#        ${CMAKE_BINARY_DIR}/Kernel/${RPI_PLATFORM}
#)

##
# FPrime instantiation
##
#set(CMAKE_DEBUG_OUTPUT TRUE)  # debug the CMake configuration
include(cmake/fprime.cmake)

# Set the framework libraries provided by FPrime
fprime_enter_root(${PROJECT_SOURCE_DIR}/fprime)
include(cmake/fprime_code.cmake)

fprime_enter_root(${CMAKE_CURRENT_LIST_DIR})

#add_subdirectory(Kernel)    # Deployment specific drivers
add_subdirectory(Rpi)   # FSW

cmake_policy(SET CMP0079 NEW)
