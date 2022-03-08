cmake_minimum_required(VERSION 3.15)

# Initialize the compiler toolchain
include(toolchain.cmake)

project(VO-CMPE460)

include(ExternalProject)
ExternalProject_Add(
        usbboot
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/usbboot
        BINARY_DIR ${CMAKE_BINARY_DIR}/usbboot
        CONFIGURE_COMMAND "${CMAKE_COMMAND}" -E make_directory ${CMAKE_BINARY_DIR}/usbboot
        BUILD_COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR}/usbboot
        INSTALL_COMMAND ""
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(AARCH 32)
set(RASPPI 1)
set(ARM_ALLOW_MULTI_CORE OFF)
include(circle/circle.cmake)

add_subdirectory(circle)
add_subdirectory(src)
