target_include_directories(firmware PRIVATE
	"${PROJECT_ROOT}/include"
)

target_link_libraries(firmware PRIVATE
	idf::freertos

	gpsDriver
	TelemetryHub
	Logger
	inputHandler
)


