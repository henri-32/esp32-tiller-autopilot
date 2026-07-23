add_library(i2cDriver STATIC 
	i2c_driver.cpp
)

add_library(gpsDriver STATIC 
	gpsDriver.cpp 
) 

add_library(compassDriver STATIC 
	compassDriver.cpp
) 

add_library(pwmDriver STATIC 
	pwmDriver.cpp
)

add_library(uartDriver STATIC 
	uartDriver.cpp
)

target_link_libraries(i2cDriver PRIVATE 
	i2c_hwconfig
)

target_link_libraries(gpsDriver PRIVATE
	idf::esp_driver_uart 
	idf::freertos
	idf::esp_timer
	minmea
	gps_hwconfig
	queueServer
)

target_link_libraries(uartDriver PRIVATE
	idf::esp_driver_uart
	cobs
)

target_link_libraries(compassDriver PRIVATE 
	i2c_hwconfig
)	

target_link_libraries(pwmDriver PRIVATE
	idf::freertos
	idf::esp_driver_gpio
	idf::esp_driver_ledc
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

target_include_directories(pwmDriver PRIVATE 
	"${PROJECT_ROOT}/include"
)

target_include_directories(uartDriver PRIVATE 
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

target_compile_options(pwmDriver PRIVATE 
	-fstack-usage
)

option(MEM_LOG "Adds logging functionality of memory/perfomance values" OFF)
if(MEM_LOG)	
	target_compile_options(i2cDriver PRIVATE 
		-fstack-usage
		-DMEM_LOG
	)

	target_compile_options(gpsDriver PRIVATE 
		-fstack-usage
		-DMEM_LOG
	)

	target_compile_options(compassDriver PRIVATE 
		-fstack-usage
		-DMEM_LOG
	)

	target_compile_options(pwmDriver PRIVATE 
		-fstack-usage
		-DMEM_LOG
	)
endif()

