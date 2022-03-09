include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(fw)
add_subdirectory(rate_driver)
add_subdirectory(i2c_mux)
add_subdirectory(ardu_cam)
add_subdirectory(dp)
add_subdirectory(evr)
add_subdirectory(prm)
add_subdirectory(dbg)

add_subdirectory(kernel)
