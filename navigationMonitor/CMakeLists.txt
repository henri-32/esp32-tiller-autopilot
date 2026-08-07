cmake_minimum_required(VERSION 3.15) 
project(navigationMonitor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

#find_package(FLTK 1.4 CONFIG REQUIRED)
add_subdirectory(
	../lib/fltk
	${CMAKE_BINARY_DIR}/fltk-build
)

add_executable(navigationMonitor 
	main.cpp
	socket_handling.cpp
	../lib/cobs-c/cobs.c
)


target_link_libraries(navigationMonitor PRIVATE
	fltk::fltk
)

target_include_directories(navigationMonitor PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/../include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../lib"
)
