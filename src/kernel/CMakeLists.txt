build_elf(vo-car main.cc)
target_link_libraries(vo-car
        rate_driver ardu_cam i2c_mux
)

add_dependencies(vo-car usbboot)
