####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####
set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/RpiTopologyAppAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/Topology.cc"
        )
# Note: supply non-explicit dependencies here
set(MOD_DEPS
        Rpi/SystemTime
        Rpi/Logger
        Os
        )

register_fprime_module()

build_elf(vo-car main.cc)
target_link_libraries(vo-car Rpi_Top)

