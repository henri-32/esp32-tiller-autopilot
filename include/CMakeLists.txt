add_library(hardwareConfig INTERFACE) 

target_include_directories(hardwareConfig
	INTERFACE
		"${PROJECT_ROOT}/include"
)

target_link_libraries(hardwareConfig 
	INTERFACE 
		idf::esp_driver_i2c
		idf::esp_driver_gpio
		idf::esp_driver_uart
		
)
