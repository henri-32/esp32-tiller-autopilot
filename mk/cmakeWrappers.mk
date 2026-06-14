SHELL := /bin/bash
SOURCE := source ./platform/esp-idf/export.sh
MONITOR:= python -m esp_idf_monitor -b 115200

fresh: 
	@rm ./build/CMakeCache.txt
	@echo "CMakeCache.txt removed..." 
	@echo "reconfigure the project with configure_*"

targets: 
	@cat maketargets.txt

configure_hardwaretest-compass: 
	@$(SOURCE) && cmake --preset hardwaretest-compass | tee .logs/lastBuild.log

build_hardwaretest-compass:  
	@$(SOURCE) && cmake --build --preset hardwaretest-compass -j8 | tee .logs/lastBuild.log

flash_hardwaretest-compass:  
	@$(SOURCE) && cmake --build --preset hardwaretest-compass -j8 --target flash | tee .logs/lastBuild.log

monitor_hardwaretest-compass: 
	@$(SOURCE) && $(MONITOR) $(CURDIR)/build/hardwaretest-compass/hardwaretest-compass.elf

