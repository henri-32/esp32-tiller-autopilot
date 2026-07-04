add_library(i2cDriver STATIC 
	i2c_driver.cpp
)

add_library(gpsDriver STATIC 
	gpsDriver.cpp 
) 

add_library(compassDriver STATIC 
	compassDriver.cpp
) 

target_link_libraries(i2cDriver PRIVATE 
	i2c_hwconfig
)

target_link_libraries(gpsDriver PRIVATE
	idf::esp_driver_uart 
	minmea
	gps_hwconfig
)

target_link_libraries(compassDriver PRIVATE 
	i2c_hwconfig
)	

target_include_directories(i2cDriver PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_include_directories(gpsDriver PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_include_directories(compassDriver PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_compile_options(i2cDriver PRIVATE 
	-fstack-usage
)

target_compile_options(gpsDriver PRIVATE 
	-fstack-usage
)

target_compile_options(compassDriver PRIVATE 
	-fstack-usage
)
