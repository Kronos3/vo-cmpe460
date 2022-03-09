build_elf(vo-car main.cc)
target_link_libraries(vo-car
        rate_driver i2c_mux
        fatfs sdcard
        ardu_cam  evr dp prm
        dbg
)

add_dependencies(vo-car usbboot)

build_elf(blinky blinky_main.cc)
target_link_libraries(blinky
        rate_driver dbg
        )
add_dependencies(blinky usbboot)
