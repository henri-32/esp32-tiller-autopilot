add_library(Logger STATIC 
	logger.cpp
) 

target_include_directories(Logger PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_link_libraries(Logger PRIVATE 
	idf::freertos
	uartDriver
)

target_compile_options(Logger PRIVATE 
	-fstack-usage
)
