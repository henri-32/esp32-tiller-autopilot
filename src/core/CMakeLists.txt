add_library(queueServer STATIC
	queueServer.cpp
)

add_library (DataStore STATIC 
	dataStore.cpp
)
add_library(inputHandler STATIC 
	inputHandler/inputHandler.cpp
)

target_link_libraries(queueServer PRIVATE 
	idf::freertos
)

target_link_libraries(DataStore PRIVATE 
	idf::freertos
)

target_link_libraries(inputHandler PRIVATE 
	uartDriver
	cobs
)

target_include_directories(queueServer PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_include_directories(DataStore PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_include_directories(inputHandler PRIVATE 
	"${PROJECT_ROOT}/include"
	"${PROJECT_ROOT}/platform"
)

target_compile_options(DataStore PRIVATE 
	-fstack-usage
)

target_compile_options(inputHandler PRIVATE 
	-fstack-usage
)
