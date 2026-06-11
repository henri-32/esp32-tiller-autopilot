SHELL := /bin/bash

flash: 
	@source ./platform/esp-idf/export.sh && cmake --build build --target flash 2>&1 | tee  .logs/build.log 
