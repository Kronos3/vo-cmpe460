set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/VideoStreamerComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/VideoStreamerImpl.cc"
        "${CMAKE_CURRENT_LIST_DIR}/output/net_output.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/output/output.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/preview/drm_preview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/encoder/h264_encoder.cpp"
        )

register_fprime_module()
