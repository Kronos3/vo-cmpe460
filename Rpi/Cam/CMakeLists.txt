set(SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/CamComponentAi.xml
        ${CMAKE_CURRENT_LIST_DIR}/CamListenerEnumAi.xml
        ${CMAKE_CURRENT_LIST_DIR}/CamImpl.cc
        ${CMAKE_CURRENT_LIST_DIR}/core/libcamera_app.cc
        )

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/Ports")

set(MOD_DEPS
#        libcamera_app
        )

message(STATUS ${OpenCV_LIBS})
register_fprime_module()
