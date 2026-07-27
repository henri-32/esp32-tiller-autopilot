cmake_minimum_required(VERSION 4.0)

project(autopilotGateway C CXX)

add_executable(autopilotGateway  
	autopilotGateway.cpp
	consume_messages.cpp
)

add_library(cobs STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/../lib/cobs-c/cobs.c
)

target_include_directories(autopilotGateway PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/../include"
      "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
)

target_include_directories(cobs PUBLIC 
      "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
)
	
target_link_libraries(autopilotGateway PRIVATE 
	cobs
)
