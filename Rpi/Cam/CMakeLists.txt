#add_subdirectory(libcamera-apps/core)
#add_subdirectory(libcamera-apps/encoder)
#add_subdirectory(libcamera-apps/image)
#add_subdirectory(libcamera-apps/output)
#add_subdirectory(libcamera-apps/preview)
#add_subdirectory(libcamera-apps/post_processing_stages)

set(SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/CamComponentAi.xml
        ${CMAKE_CURRENT_LIST_DIR}/CamImpl.cc
        ${CMAKE_CURRENT_LIST_DIR}/core/libcamera_app.cc
#        ${CMAKE_CURRENT_LIST_DIR}/core/post_processor.cpp

#        ${CMAKE_CURRENT_LIST_DIR}/preview/drm_preview.cpp
#        ${CMAKE_CURRENT_LIST_DIR}/preview/preview.cpp
        )

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Ports")

set(MOD_DEPS
#        libcamera_app
        )

message(STATUS ${OpenCV_LIBS})
register_fprime_module()
