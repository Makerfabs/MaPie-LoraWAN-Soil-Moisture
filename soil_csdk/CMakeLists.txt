cmake_minimum_required(VERSION 3.22.1)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
project(my_project)

pico_sdk_init()
add_executable(main main.c aht10.c)
target_link_libraries(main 
    pico_stdlib pico_runtime
    hardware_adc hardware_pwm hardware_uart 
    hardware_i2c hardware_sleep hardware_rtc)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)


