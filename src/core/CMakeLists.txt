add_library(queueServer STATIC
	queueServer.cpp
)

add_library (DataStore STATIC 
	dataStore.cpp
)

target_link_libraries(queueServer PRIVATE 
	idf::freertos
)

target_link_libraries(DataStore PRIVATE 
	idf::freertos
)

target_include_directories(queueServer PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_include_directories(DataStore PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_compile_options(DataStore PRIVATE 
	-fstack-usage
)
