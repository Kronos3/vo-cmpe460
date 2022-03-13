set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/ardu_cam.cc"
        "${CMAKE_CURRENT_LIST_DIR}/ardu_cam_i2c.cc"
        "${CMAKE_CURRENT_LIST_DIR}/ov2640.cc"
        "${CMAKE_CURRENT_LIST_DIR}/ov5642.cc"
        "${CMAKE_CURRENT_LIST_DIR}/CamImpl.cc"
        "${CMAKE_CURRENT_LIST_DIR}/CamComponentAi.xml"
)

set(MOD_DEPS
        Drv/I2cMux
        Drv/RpiSpiDriver
        Rpi/FrameBuffer
)

register_fprime_module()
