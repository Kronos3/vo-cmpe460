####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
# Note: using PROJECT_NAME as EXECUTABLE_NAME
####
set(SOURCE_FILES
        "${CMAKE_CURRENT_LIST_DIR}/RpiRTLinuxTopologyAppAi.xml"
        "${CMAKE_CURRENT_LIST_DIR}/Topology.cc"
        "${CMAKE_CURRENT_LIST_DIR}/main.cc"
        #        "${CMAKE_CURRENT_LIST_DIR}/backtrace.c"
        )
# Note: supply non-explicit dependencies here
set(MOD_DEPS
#        Rpi/SystemTime
#        Rpi/Logger
        Drv/TcpClient
        Svc/LinuxTime
        )

set(EXECUTABLE_NAME car)
register_fprime_executable()

set(OPENCV_LIBRARIES
#        ${PROJECT_SOURCE_DIR}/opencv/libopencv_photo.a
#        ${PROJECT_SOURCE_DIR}/opencv/libopencv_video.a
#        ${PROJECT_SOURCE_DIR}/opencv/libopencv_videoio.a
        ${PROJECT_SOURCE_DIR}/opencv/libopencv_imgcodecs.a
#        ${PROJECT_SOURCE_DIR}/opencv/libopencv_imgproc.a
#        ${PROJECT_SOURCE_DIR}/opencv/libopencv_calib3d.a
#        ${PROJECT_SOURCE_DIR}/opencv/libopencv_flann.a
#        ${PROJECT_SOURCE_DIR}/opencv/libopencv_features2d.a
#        ${PROJECT_SOURCE_DIR}/opencv/libopencv_core.a
#        ${PROJECT_SOURCE_DIR}/opencv/opencv4/3rdparty/liblibpng.a
#        ${PROJECT_SOURCE_DIR}/opencv/opencv4/3rdparty/liblibjpeg-turbo.a
        ${PROJECT_SOURCE_DIR}/opencv/opencv4/3rdparty/libzlib.a
        )

target_link_libraries(car_exe PUBLIC
        pthread

        # Work through dependencies
        ${OPENCV_LIBRARIES}
        ${OPENCV_LIBRARIES}
#        opencv_imgcodecs
        opencv_imgproc
        opencv_calib3d
        opencv_core
        camera
        camera-base
)
