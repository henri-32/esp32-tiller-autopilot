target_include_directories(compassModuleHardwareTest.elf PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_ROOT}/include"	
	"${PROJECT_ROOT}/platform/esp-idf/components/esp_driver_gpio/include"
	"${PROJECT_ROOT}/platform/esp-idf/components/freertos/FreeRTOS-Kernel/include"
)
target_link_libraries(compassModuleHardwareTest.elf PRIVATE
	idf::freertos
	idf::esp_driver_gpio
	idf::esp_system
)
		
