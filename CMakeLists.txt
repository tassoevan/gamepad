cmake_minimum_required(VERSION 3.27)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

project(gamepad VERSION 1.0.0)

add_subdirectory(lib/ws2812)

add_executable(gamepad)

target_sources(gamepad PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src/main.c
  ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
  ${CMAKE_CURRENT_LIST_DIR}/src/gamepad.c
)

target_include_directories(gamepad PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(gamepad PRIVATE pico_stdlib pico_unique_id tinyusb_device tinyusb_board ws2812)

pico_add_extra_outputs(gamepad)

