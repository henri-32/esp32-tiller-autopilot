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
	@cp build/hardwaretest-compass/compile_commands.json compile_commands.json

build_hardwaretest-compass:  
	@$(SOURCE) && cmake --build --preset hardwaretest-compass -j8 | tee .logs/lastBuild.log
	@cp build/hardwaretest-compass/compile_commands.json compile_commands.json

flash_hardwaretest-compass:  
	@$(SOURCE) && cmake --build --preset hardwaretest-compass -j8 --target flash | tee .logs/lastBuild.log

monitor_hardwaretest-compass: 
	@mkdir -p .logs
	@$(SOURCE) && $(MONITOR) $(CURDIR)/build/hardwaretest-compass/hardwaretest-compass.elf 2>&1 | \
	tee >(sed -u $$'s/\x1b\\[[0-9;]*m//g' > .logs/lastMonitor.log)

configure_hardwaretest-gps: 
	@$(SOURCE) && cmake --preset hardwaretest-gps | tee .logs/lastBuild.log
	@cp build/hardwaretest-gps/compile_commands.json compile_commands.json

build_hardwaretest-gps: 
	@$(SOURCE) && cmake --build --preset hardwaretest-gps -j8 | tee .logs/lastBuild.log 
	@cp build/hardwaretest-gps/compile_commands.json compile_commands.json

flash_hardwaretest-gps: 
	@$(SOURCE) && cmake --build --preset hardwaretest-gps -j8 --target flash | tee .logs/lastBuild.log 

monitor_hardwaretest-gps: 
	@mkdir -p .logs
	@$(SOURCE) && $(MONITOR) $(CURDIR)/build/hardwaretest-gps/hardwaretest-gps.elf 2>&1 | \
	tee >(sed -u $$'s/\x1b\\[[0-9;]*m//g' > .logs/lastMonitor.log) 
