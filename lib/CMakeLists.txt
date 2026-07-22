add_library(minmea STATIC 
	minmea/minmea.c
) 

target_include_directories(minmea INTERFACE 
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(cobs STATIC 
	cobs-c/cobs.c
)

target_include_directories(cobs INTERFACE 
	"${CMAKE_CURRENT_SOURCE_DIR}"
)

