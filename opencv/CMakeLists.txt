set(OPENCV_LIBS ${CMAKE_CURRENT_LIST_DIR}/bin/libopencv_core.a)
set(OPENCV_INCLUDE ${PROJECT_SOURCE_DIR}/opencv/)

add_library(opencv INTERFACE)
target_link_libraries(
        opencv INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/3rdparty/libzlib.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/libopencv_calib3d.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/libopencv_core.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/libopencv_calib3d.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/libopencv_features2d.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/libopencv_imgcodecs.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/libopencv_imgproc.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/libopencv_video.a
        ${CMAKE_CURRENT_LIST_DIR}/lib/opencv4/libopencv_videoio.a
)

target_include_directories(
        opencv INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include/opencv4
)
