add_library(i2c_hwconfig INTERFACE) 

target_link_libraries(i2c_hwconfig INTERFACE
		idf::esp_driver_i2c
)

add_library(gps_hwconfig INTERFACE) 

target_link_libraries(gps_hwconfig INTERFACE 
	idf::esp_driver_uart
)

		
