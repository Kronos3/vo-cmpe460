add_library(rate_driver STATIC rate_driver.cc rate_group.cc)
target_link_libraries(rate_driver circle fw)
