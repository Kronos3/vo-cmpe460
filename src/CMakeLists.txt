include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(utils)
add_subdirectory(i2c_mux)
add_subdirectory(ardu_cam)

add_subdirectory(dbg)

add_subdirectory(Test)

add_subdirectory(SystemTime)
add_subdirectory(RPISerialDriver)
add_subdirectory(RPI)
