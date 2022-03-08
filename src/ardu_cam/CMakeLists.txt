add_library(ardu_cam STATIC
        ardu_cam.cc ardu_cam_i2c.cc
        ov2640.cc ov5642.cc
        ardu_cam.h common.h
        ov5642_regs.h ov2640_regs.h)
target_link_libraries(ardu_cam fw i2c_mux circle)
