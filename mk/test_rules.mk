$(TEST_BIN): $(TEST_OBJS) $(SRC_UNDER_TEST_OBJS) $(LIB_OBJS) 
	@$(CXX) $(TEST_LD_FLAGS)  $^ -o $@ 
	@chmod +x $(TEST_BIN)
	@echo 'LINK $@' 
	@echo 'Build Mode = $(MODE)'

$(TEST_BUILD_DIR)/test/%.o: test/%.cpp
	@mkdir -p $(dir $@) 
	@$(CACHED_CXX) $(TEST_CXX_FLAGS) -c $< $(TEST_INCLUDE) $(LIB_INCLUDE) -o $@
	@echo 'CACHED_CXX $< -> $@'

$(TEST_BUILD_DIR)/src/%.o: src/%.cpp
	@mkdir -p $(dir $@) 
	@$(CACHED_CXX) $(TEST_CXX_FLAGS) -c $< $(TEST_INCLUDE) -o $@ 
	@echo 'CACHED_CXX $< -> $@'
