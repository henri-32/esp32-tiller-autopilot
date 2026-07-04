target_include_directories(hardwaretest-gps.elf PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_ROOT}/include"
)

target_link_libraries(hardwaretest-gps.elf PRIVATE 
	idf::esp_driver_gpio
	idf::esp_driver_i2c
	idf::esp_driver_uart
	idf::esp_timer

	gpsDriver
	hardwaretest_runner
)

target_compile_options(hardwaretest-gps.elf PRIVATE
	-fstack-usage
)
