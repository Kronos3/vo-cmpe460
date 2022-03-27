set(SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/CamComponentAi.xml
        ${CMAKE_CURRENT_LIST_DIR}/CamImpl.cc
        )

register_fprime_module()
target_link_libraries(Rpi_Cam PUBLIC camera)
target_include_directories(Rpi_Cam PRIVATE /usr/aarch64-linux-gnu/include/libcamera/)
