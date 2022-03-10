set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/RpiSerialDriverComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/RpiSerialDriverImpl.cc"
        )

set(MOD_DEPS circle)
register_fprime_module()
