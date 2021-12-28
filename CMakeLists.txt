cmake_minimum_required(VERSION 3.22.0)

include(external/PicoSDK/pico_sdk_import.cmake)

project(PicoGameboy)

set(CMAKE_TOOLCHAIN_FILE C:/Program Files (x86)/GNU Arm Embedded Toolchain/10 2021.10/bin/arm-none-eabi-objcopy.exe)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
enable_language(C CXX)

pico_sdk_init()

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(external/st7789)

target_link_libraries(${PROJECT_NAME}
	private pico_st7789
)