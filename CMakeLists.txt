cmake_minimum_required(VERSION 3.22.0)

include(external/pimoroni-pico/pico_sdk_import.cmake)
include(external/pimoroni-pico/libraries/pico_explorer/pico_explorer.cmake)

pico_sdk_init()

project(PicoGameboy)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
	src/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
	pico_stdlib
	pico_explorer
)

