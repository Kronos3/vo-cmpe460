add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Ports")
add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Types")

set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/MotComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/MotImpl.cc"
        )

set(MOD_DEPS
        Rpi/Mot/Types
        Rpi/Mot/Ports
)

register_fprime_module()
