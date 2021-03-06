# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(matrixscan C CXX ASM)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(matrixscan
	main.c
	matrixscan.c
	layer.c
	usb_descriptors.c
	hid.c
	)

# Link Time Optimization is not supported yet.
#set_property(TARGET matrixscan PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

pico_set_program_name(matrixscan "matrixscan")
pico_set_program_version(matrixscan "0.1")

pico_enable_stdio_uart(matrixscan 1)
pico_enable_stdio_usb(matrixscan 0)

target_include_directories(matrixscan PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
	)

target_link_libraries(matrixscan
	pico_stdlib
	tinyusb_device
	tinyusb_board
	)

pico_add_extra_outputs(matrixscan)
