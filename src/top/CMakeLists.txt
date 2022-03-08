build_elf(vo-car main.cc)
target_link_libraries(vo-car
        rate_driver
)

add_dependencies(vo-car usbboot)
