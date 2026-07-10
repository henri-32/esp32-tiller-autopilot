target_include_directories(hardwaretest-pwm.elf PRIVATE
	${PROJECT_ROOT}/include 
)

target_link_libraries(hardwaretest-pwm.elf PRIVATE 
	idf::freertos	
	idf::esp_driver_gpio
	idf::esp_driver_ledc
	hardwaretest_runner
	pwmDriver
)
