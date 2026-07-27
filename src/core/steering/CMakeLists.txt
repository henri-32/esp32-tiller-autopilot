add_library(csc STATIC 
	csc/csc.cpp
	csc/headingErrorCalculator.cpp
	csc/observationBuffer.cpp
	csc/steeringGuard.cpp
	csc/deadband.cpp
)

target_include_directories(csc PRIVATE
	"${PROJECT_ROOT}/include"
) 

target_link_libraries(csc PRIVATE
	idf::freertos
)

