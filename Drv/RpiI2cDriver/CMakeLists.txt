set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/RpiI2cDriverImpl.cc"
        "${CMAKE_CURRENT_LIST_DIR}/RpiI2cDriverComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/RpiI2cStatusEnumAi.xml"
        )

register_fprime_module()
