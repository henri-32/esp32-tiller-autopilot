cmake_minimum_required(VERSION 3.22) 

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
message(STATUS "PROJECT_ROOT = ${PROJECT_ROOT}")
set(IDF_PATH "${PROJECT_ROOT}/platform/esp-idf")
set(ENV{IDF_PATH} "${IDF_PATH}")
message(STATUS "IDF_PATH = ${IDF_PATH}")
set(IDF_TARGET "esp32" CACHE STRING "ESP-IDF mcu target") 
message(STATUS "IDF_TARGET = ${IDF_TARGET}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(CMAKE_TOOLCHAIN_FILE 
	"${IDF_PATH}/tools/cmake/toolchain-${IDF_TARGET}.cmake"
	CACHE FILEPATH "ESP-IDF toolchain file" 
) 
message(STATUS "TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")

project(autopilot C CXX ASM) 

include("${IDF_PATH}/tools/cmake/idf.cmake")

idf_build_process("${IDF_TARGET}") 

option(BUILD_COMPASS_HARDWARE_TEST "Build path for compass hardware tests" ON)
if (BUILD_COMPASS_HARDWARE_TEST)
	add_executable(compassModuleHardwareTest.elf 
		test/hardwaretests/compassModuleHardwareTest/compassModuleHardwareTest.cpp
	)
	add_subdirectory(test/hardwaretests/compassModuleHardwareTest)
	idf_build_executable(compassModuleHardwareTest.elf)
endif()

