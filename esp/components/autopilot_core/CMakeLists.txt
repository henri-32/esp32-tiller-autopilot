set(AUTOPILOT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../..") 

idf_component_register( 
	SRCS 
		"${AUTOPILOT_ROOT}/src/core/systemController.cpp" 
		"${AUTOPILOT_ROOT}/src/core/steering/csc/csc.cpp"
		"${AUTOPILOT_ROOT}/src/core/steering/csc/deadband.cpp" 
		"${AUTOPILOT_ROOT}/src/core/steering/csc/headingErrorCalculator.cpp"
		"${AUTOPILOT_ROOT}/src/core/steering/csc/observationBuffer.cpp"
		"${AUTOPILOT_ROOT}/src/core/steering/csc/steeringGuard.cpp" 

	INCLUDE_DIRS
		"${AUTOPILOT_ROOT}/include"
)
