cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)
# include(pico_extras_import.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(gamepad VERSION 1.0.0)

add_executable(gamepad src/main.c)

target_link_libraries(gamepad PRIVATE pico_stdlib)

pico_sdk_init()

pico_enable_stdio_usb(gamepad 1)
pico_enable_stdio_uart(gamepad 1)

pico_add_extra_outputs(gamepad)

