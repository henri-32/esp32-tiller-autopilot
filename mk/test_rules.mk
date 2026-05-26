$(TEST_BIN): $(TEST_OBJS) $(SRC_UNDER_TEST_OBJS) $(LIB_OBJS) 
	@$(CXX) $(LD_FLAGS)  $^ -o $@ 
	@echo 'LINK $@' 


$(TEST_BUILD_DIR)/test/%.o: test/%.cpp
	@mkdir -p $(dir $@) 
	@$(CACHED_CXX) $(TEST_CXX_FLAGS) -c $< $(TEST_INCLUDE) $(LIB_INCLUDE) -o $@
	@echo 'CACHED_CXX $< -> $@'

$(TEST_BUILD_DIR)/src/%.o: src/%.cpp
	@mkdir -p $(dir $@) 
	@$(CACHED_CXX) $(TEST_CXX_FLAGS) -c $< $(TEST_INCLUDE) -o$@ 
	@echo 'CACHED_CXX $< -> $@'
