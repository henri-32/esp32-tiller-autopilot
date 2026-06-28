target_include_directories(hardwaretest-gps.elf PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_ROOT}/include"
)

target_link_libraries(hardwaretest-gps.elf PRIVATE 
	idf::freertos
	idf::esp_driver_gpio
	idf::esp_system
    idf::esp_driver_i2c
	hardwaretest_runner
)
