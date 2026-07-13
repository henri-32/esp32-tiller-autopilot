cmake_minimum_required(VERSION 3.15) 
project(nativeDisplay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

#find_package(FLTK 1.4 CONFIG REQUIRED)
add_subdirectory(
	../lib/fltk
	${CMAKE_BINARY_DIR}/fltk-build
)


add_executable(nativeDisplay 
	nativeDisplay.cpp
)

target_link_libraries(nativeDisplay PRIVATE
	fltk::fltk
)
