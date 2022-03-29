set(SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/CamComponentAi.xml
        ${CMAKE_CURRENT_LIST_DIR}/CamImpl.cc
        ${CMAKE_CURRENT_LIST_DIR}/LibCamera.cpp
        )

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Ports")

message(STATUS ${OpenCV_LIBS})
register_fprime_module()
target_include_directories(Rpi_Cam PUBLIC /usr/aarch64-linux-gnu/include/libcamera/)
