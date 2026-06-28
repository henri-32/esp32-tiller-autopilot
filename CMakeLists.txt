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

option(BUILD_HARDWARETEST "Build path for hardwaretests in general" OFF)
if (BUILD_HARDWARETEST)
	add_subdirectory(test/hardwaretest)
endif()

option(BUILD_COMPASS_HARDWARE_TEST "Build path for compass hardware tests" OFF)
if (BUILD_COMPASS_HARDWARE_TEST)
	add_executable(hardwaretest-compass.elf 
		test/hardwaretest/hardwaretest-compass/hardwaretest-compass.cpp
		src/drivers/compassDriver.cpp 
		src/drivers/i2c_driver.cpp
	)
	add_subdirectory(test/hardwaretest/hardwaretest-compass)

	idf_build_executable(hardwaretest-compass.elf)

	target_link_libraries(hardwaretest-compass.elf PRIVATE
		idf::freertos
		hardwaretest_runner
	)
		
endif()

