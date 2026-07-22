cmake_minimum_required(VERSION 4.0)

project(logRouter C CXX)

add_executable(logRouter  
	logRouter.cpp
)

add_library(cobs STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/../lib/cobs-c/cobs.c
)

target_include_directories(logRouter PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/../include"
      "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
)

target_include_directories(cobs PUBLIC 
      "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
)
	
target_link_libraries(logRouter PRIVATE 
	cobs
)
