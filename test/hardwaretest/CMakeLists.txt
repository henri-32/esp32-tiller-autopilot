add_library(hardwaretest_runner STATIC 
	hardwaretest_runner.cpp
)

target_link_libraries(hardwaretest_runner PRIVATE
	idf::freertos
)

target_compile_options(
	hardwaretest_runner 
	PRIVATE 
		-fstack-usage
)

