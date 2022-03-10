set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/RPISerialDriverComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/RPISerialDriverImpl.cc"
        )

set(MOD_DEPS circle)
register_fprime_module()
