set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/VideoStreamerComponentAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/VideoStreamerImpl.cc"
#        "${CMAKE_CURRENT_LIST_DIR}/PracticalSocket.cc"
#        "${CMAKE_CURRENT_LIST_DIR}/output/net_output.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/output/output.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/preview/drm_preview.cpp"
        )

register_fprime_module()
target_link_libraries(Rpi_VideoStreamer PUBLIC
        /usr/aarch64-linux-gnu/lib/libdrm.so.2)
