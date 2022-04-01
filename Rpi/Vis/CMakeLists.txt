set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/VisImpl.cc"
        "${CMAKE_CURRENT_LIST_DIR}/VisPipeline.cc"
        "${CMAKE_CURRENT_LIST_DIR}/VisComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/VisListenerEnumAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/VisPipeEnumAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/VisRecord.cc"
        )

register_fprime_module()
