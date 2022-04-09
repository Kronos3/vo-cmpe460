set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/NavImpl.cc"
        "${CMAKE_CURRENT_LIST_DIR}/NavSimple.cc"
        "${CMAKE_CURRENT_LIST_DIR}/NavComplex.cc"
        "${CMAKE_CURRENT_LIST_DIR}/NavAlgorithm.cc"
        "${CMAKE_CURRENT_LIST_DIR}/NavComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/NavAlgorithmTypeEnumAi.xml"
        )

register_fprime_module()
