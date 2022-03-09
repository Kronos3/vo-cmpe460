add_library(fw STATIC fw.cc fw_time.cc serializable.cc circular_buffer.cc)
target_link_libraries(fw circle)
