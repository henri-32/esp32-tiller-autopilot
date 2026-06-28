target_include_directories(hardwaretest-compass.elf PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_ROOT}/include"	
)

target_link_libraries(hardwaretest-compass.elf PRIVATE
	idf::esp_driver_gpio
	idf::esp_driver_i2c

	compassDriver
	i2cDriver
	hardwaretest_runner
)
		
