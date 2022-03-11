set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/RpiSpiDriverImpl.cc"
        "${CMAKE_CURRENT_LIST_DIR}/RpiSpiDriverComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/SpiDmaReplyPortAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/SpiStatusEnumAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/SpiTransactPortAi.xml"
)

register_fprime_module()
