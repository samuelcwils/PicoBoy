cmake_minimum_required(VERSION 3.22.0)

include(external/pico_sdk_import.cmake)
include(external/pimoroni_pico_import.cmake)
include(${PIMORONI_PICO_PATH}/libraries/pico_explorer/pico_explorer.cmake)

project(PicoGameboy)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/bus.cpp
	src/cart.cpp
	src/ppu.cpp
	src/sound.cpp
	src/MBC1.cpp
)

include_directories(${PROJECT_NAME}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/incbin
)


target_link_libraries(${PROJECT_NAME} 
	pico_stdlib
	pico_multicore
	pico_explorer
)


pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(PicoGameboy)