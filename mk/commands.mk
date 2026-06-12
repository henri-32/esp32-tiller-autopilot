SHELL := /bin/bash
SOURCE := source ./platform/esp-idf/export.sh

targets: 
	@cat maketargets.txt

configure_hardwaretest-compass: 
	@$(SOURCE) && cmake --preset hardwaretest-compass

build_hardwaretest-compass:  
	@$(SOURCE) && cmake --build --preset hardwaretest-compass -j8

flash_hardwaretest-compass:  
	@$(SOURCE) && cmake --build --preset hardwaretest-compass -j8 --target flash

