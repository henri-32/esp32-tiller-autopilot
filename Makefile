include mk/global_variables.mk
include mk/toolchain.mk
include mk/test_variables.mk
include mk/test_lib_variables.mk
include mk/test_lib_rules.mk
include mk/test_rules.mk 

.DEFAULTTARGET= test

.PHONY: test
test: $(TEST_BIN) 

.PHONY: compiledb_test
compiledb_test: 
	@$(MAKE) clean_test
	@bear -- $(MAKE) test

.PHONY: clean 
clean: 
	@rm -rf build_test 
	@rm -rf build_lib 
	@echo 'removed all build artefacts'

.PHONY: clean_test 
clean_test: 
	@rm -rf build_test 
	@echo 'removed test binary and build artefacts. NO library object files removed'

.PHONY: clean_lib
clean_lib:
	@rm -rf build_lib 
	@echo 'removed library object files'


