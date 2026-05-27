include mk/global_variables.mk
include mk/toolchain.mk
include mk/test_variables.mk
include mk/test_lib_variables.mk
include mk/test_lib_rules.mk
include mk/test_rules.mk 

.DEFAULTTARGET= test

.PHONY: test
test: $(TEST_BIN) 

.PHONY: run_test
run_test: 
	./$(TEST_BIN)

.PHONY: compiledb_test
compiledb_test: 
	@$(MAKE) -no-print-directory clean_test
	@bear -- $(MAKE) -no-print-directory test

.PHONY: clean 
clean: 
	@rm -rf build_test 
	@echo 'removed all build artefacts (Libs excluded. To remove lib artefacts run "make clean_lib"'

.PHONY: clean_test 
clean_test: 
	@rm -rf build_test 
	@echo 'removed test binary and build artefacts. NO library object files removed'

.PHONY: clean_lib
clean_lib:
	@rm -rf build_lib 
	@echo 'removed library object files'


