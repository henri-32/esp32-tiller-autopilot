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

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(src/core)
add_subdirectory(src/drivers) 
add_subdirectory(src/logging)

option(BUILD_HARDWARETEST "Build path for hardwaretests in general" OFF)
if (BUILD_HARDWARETEST)
	add_subdirectory(test/hardwaretest)
endif()

option(BUILD_COMPASS_HARDWARETEST "Build path for compass hardware tests" OFF)
if (BUILD_COMPASS_HARDWARETEST)
	add_executable(hardwaretest-compass.elf 
		test/hardwaretest/hardwaretest-compass/hardwaretest-compass.cpp
	)
	add_subdirectory(test/hardwaretest/hardwaretest-compass)

	idf_build_executable(hardwaretest-compass.elf)
		
endif()

option(BUILD_GPS_HARDWARETEST "Build path for gps hardware tests" OFF) 
if (BUILD_GPS_HARDWARETEST)
	add_executable(hardwaretest-gps.elf
		test/hardwaretest/hardwaretest-gps/hardwaretest-gps.cpp
	)
	add_subdirectory(test/hardwaretest/hardwaretest-gps) 

	idf_build_executable(hardwaretest-gps.elf)
 
endif()

option(BUILD_PWM_HARDWARETEST "Buildpath for pwm hardware test" OFF)
if (BUILD_PWM_HARDWARETEST)
	add_executable(hardwaretest-pwm.elf
		test/hardwaretest/hardwaretest-pwm/hardwaretest-pwm.cpp
	)
	add_subdirectory(test/hardwaretest/hardwaretest-pwm)

	idf_build_executable(hardwaretest-pwm.elf)

endif()

option(BUILD_FIRMARE "Buildpath for production firmware" OFF)
if (BUILD_FIRMWARE)
	add_executable(firmware 
		src/main/main.cpp 
	)
	add_subdirectory(src/main)
	idf_build_executable(firmware)
endif()
