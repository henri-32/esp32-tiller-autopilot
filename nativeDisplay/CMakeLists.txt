cmake_minimum_required(VERSION 3.15) 
project(nativeDisplay LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

#find_package(FLTK 1.4 CONFIG REQUIRED)
add_subdirectory(
	../lib/fltk
	${CMAKE_BINARY_DIR}/fltk-build
)

add_executable(nativeDisplay 
	nativeDisplay.cpp
	../lib/cobs-c/cobs.c
)


target_link_libraries(nativeDisplay PRIVATE
	fltk::fltk
)

target_include_directories(nativeDisplay PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/../include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../lib"
)

