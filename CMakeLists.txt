cmake_minimum_required(VERSION 3.22.0)

include(external/pico_sdk_import.cmake)
include(external/pimoroni_pico_import.cmake)
include(${PIMORONI_PICO_PATH}/libraries/pico_explorer/pico_explorer.cmake)

pico_sdk_init()

project(PicoGameboy)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
	src/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
	pico_stdlib
	pico_multicore
	pico_explorer
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
