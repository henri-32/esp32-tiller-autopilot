add_library(queueServer STATIC
	queueServer.cpp
)

target_link_libraries(queueServer PRIVATE 
	idf::freertos
)

target_include_directories(queueServer PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_compile_options(queueServer PRIVATE 
	-fstack-usage
)
